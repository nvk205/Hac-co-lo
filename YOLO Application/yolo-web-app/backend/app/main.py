import os
import uuid
from contextlib import asynccontextmanager
from typing import Optional

from dotenv import load_dotenv
from fastapi import Depends, FastAPI, File, HTTPException, Query, UploadFile
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles

from app.detection import Detector
from app.schemas import DetectionResult, HealthStatus, VideoDetectionResult

load_dotenv()

# --- Config -----------------------------------------------------------------

ALLOWED_IMAGE_TYPES = {"image/jpeg", "image/jpg", "image/png"}
ALLOWED_VIDEO_TYPES = {"video/mp4"}
MAX_IMAGE_SIZE = 10 * 1024 * 1024  # 10 MB
MAX_VIDEO_SIZE = 50 * 1024 * 1024  # 50 MB

OUTPUT_DIR = os.getenv("OUTPUT_DIR", "outputs")
MODEL_NAME = os.getenv("YOLO_MODEL", "yolo11n.pt")
FRONTEND_ORIGIN = os.getenv("FRONTEND_ORIGIN", "http://localhost:5173")

os.makedirs(OUTPUT_DIR, exist_ok=True)

_detector: Optional[Detector] = None


# --- Lifespan: load the model ONCE at startup -------------------------------


@asynccontextmanager
async def lifespan(app: FastAPI):
    global _detector
    _detector = Detector(model_name=MODEL_NAME)
    yield
    _detector = None


app = FastAPI(
    title="YOLO Detection API",
    description="Backend for the Week 2 YOLO web application assignment.",
    version="1.0.0",
    lifespan=lifespan,
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=[FRONTEND_ORIGIN],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Serve annotated images/videos so the frontend can fetch them by URL
app.mount("/outputs", StaticFiles(directory=OUTPUT_DIR), name="outputs")


def get_detector() -> Detector:
    """Dependency so tests can override this without touching the real model."""
    if _detector is None:
        raise HTTPException(status_code=503, detail="Model is not loaded yet.")
    return _detector


# --- Routes -------------------------------------------------------------


@app.get("/health", response_model=HealthStatus)
def health_check():
    return {"status": "ok", "model_loaded": _detector is not None}


@app.post("/api/detect/image", response_model=DetectionResult)
async def detect_image(
    file: UploadFile = File(...),
    confidence: float = Query(0.25, ge=0.0, le=1.0),
    detector: Detector = Depends(get_detector),
):
    if file.content_type not in ALLOWED_IMAGE_TYPES:
        raise HTTPException(
            status_code=400,
            detail=f"Unsupported file type '{file.content_type}'. Allowed: JPG, JPEG, PNG.",
        )

    contents = await file.read()
    if len(contents) == 0:
        raise HTTPException(status_code=400, detail="Uploaded file is empty.")
    if len(contents) > MAX_IMAGE_SIZE:
        raise HTTPException(status_code=400, detail="Image exceeds the 10MB size limit.")

    tmp_path = os.path.join(OUTPUT_DIR, f"tmp_{uuid.uuid4().hex}.jpg")
    with open(tmp_path, "wb") as f:
        f.write(contents)

    try:
        result = detector.detect_image(tmp_path, OUTPUT_DIR, conf=confidence)
    except Exception as exc:  # keep the server alive on any inference failure
        raise HTTPException(status_code=500, detail=f"Detection failed: {exc}") from exc
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

    return result


@app.post("/api/detect/video", response_model=VideoDetectionResult)
async def detect_video(
    file: UploadFile = File(...),
    confidence: float = Query(0.25, ge=0.0, le=1.0),
    detector: Detector = Depends(get_detector),
):
    if file.content_type not in ALLOWED_VIDEO_TYPES:
        raise HTTPException(
            status_code=400,
            detail=f"Unsupported file type '{file.content_type}'. Only MP4 is supported.",
        )

    contents = await file.read()
    if len(contents) == 0:
        raise HTTPException(status_code=400, detail="Uploaded file is empty.")
    if len(contents) > MAX_VIDEO_SIZE:
        raise HTTPException(status_code=400, detail="Video exceeds the 50MB size limit.")

    tmp_path = os.path.join(OUTPUT_DIR, f"tmp_{uuid.uuid4().hex}.mp4")
    with open(tmp_path, "wb") as f:
        f.write(contents)

    try:
        result = detector.detect_video(tmp_path, OUTPUT_DIR, conf=confidence)
    except Exception as exc:
        raise HTTPException(status_code=500, detail=f"Video detection failed: {exc}") from exc
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

    return result
