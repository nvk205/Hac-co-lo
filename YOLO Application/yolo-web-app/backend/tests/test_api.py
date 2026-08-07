"""Backend tests.

These use a FakeDetector to avoid downloading real YOLO weights or needing
network access during test runs -- get_detector is overridden as a FastAPI
dependency, and the real model is never loaded because the app's lifespan
startup hook only runs when the TestClient is used as a context manager
(which we deliberately avoid here).
"""

import io

from fastapi.testclient import TestClient
from PIL import Image

from app.main import app, get_detector


class FakeDetector:
    """Stands in for app.detection.Detector so tests don't need real weights."""

    def detect_image(self, image_path, output_dir, conf=0.25):
        return {
            "detections": [
                {"class_name": "person", "confidence": 0.95, "bbox": [10.0, 10.0, 100.0, 100.0]}
            ],
            "count": 1,
            "processing_time": 0.01,
            "annotated_image_url": "/outputs/fake.jpg",
        }

    def detect_video(self, video_path, output_dir, conf=0.25):
        return {
            "frame_count": 10,
            "class_counts": {"person": 5},
            "processing_time": 0.1,
            "annotated_video_url": "/outputs/fake.mp4",
        }


app.dependency_overrides[get_detector] = lambda: FakeDetector()
client = TestClient(app)


def _sample_jpeg_bytes() -> bytes:
    image = Image.new("RGB", (64, 64), color=(255, 0, 0))
    buffer = io.BytesIO()
    image.save(buffer, format="JPEG")
    return buffer.getvalue()


def test_health_check_returns_ok():
    response = client.get("/health")
    assert response.status_code == 200
    body = response.json()
    assert body["status"] == "ok"


def test_detect_image_valid_upload_returns_detections():
    files = {"file": ("test.jpg", _sample_jpeg_bytes(), "image/jpeg")}
    response = client.post("/api/detect/image", files=files)

    assert response.status_code == 200
    data = response.json()
    assert data["count"] == 1
    assert data["detections"][0]["class_name"] == "person"
    assert "annotated_image_url" in data


def test_detect_image_rejects_unsupported_file_type():
    files = {"file": ("test.txt", b"this is not an image", "text/plain")}
    response = client.post("/api/detect/image", files=files)

    assert response.status_code == 400
    assert "Unsupported file type" in response.json()["detail"]


def test_detect_image_rejects_empty_file():
    files = {"file": ("empty.jpg", b"", "image/jpeg")}
    response = client.post("/api/detect/image", files=files)

    assert response.status_code == 400
    assert "empty" in response.json()["detail"].lower()


def test_detect_video_rejects_unsupported_file_type():
    files = {"file": ("clip.avi", b"not a real video", "video/x-msvideo")}
    response = client.post("/api/detect/video", files=files)

    assert response.status_code == 400


def test_detect_video_valid_upload_returns_summary():
    # FakeDetector.detect_video never actually decodes the bytes, so any
    # non-empty payload with the right content-type is enough here.
    files = {"file": ("clip.mp4", b"fake mp4 bytes", "video/mp4")}
    response = client.post("/api/detect/video", files=files)

    assert response.status_code == 200
    data = response.json()
    assert data["frame_count"] == 10
    assert data["class_counts"]["person"] == 5
    assert "annotated_video_url" in data


def test_detect_image_rejects_oversized_file(monkeypatch):
    import app.main as main_module

    monkeypatch.setattr(main_module, "MAX_IMAGE_SIZE", 10)  # 10 bytes, easy to exceed
    files = {"file": ("test.jpg", _sample_jpeg_bytes(), "image/jpeg")}
    response = client.post("/api/detect/image", files=files)

    assert response.status_code == 400
    assert "size limit" in response.json()["detail"].lower()
