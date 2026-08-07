"""Pydantic response models used for request validation and OpenAPI docs."""

from typing import Dict, List

from pydantic import BaseModel, Field


class Detection(BaseModel):
    class_name: str = Field(..., description="Detected object class, e.g. 'person'")
    confidence: float = Field(..., ge=0.0, le=1.0, description="Model confidence score")
    bbox: List[float] = Field(
        ..., description="Bounding box as [x_min, y_min, x_max, y_max] in pixels"
    )


class DetectionResult(BaseModel):
    detections: List[Detection]
    count: int = Field(..., description="Total number of objects detected")
    processing_time: float = Field(..., description="Inference time in seconds")
    annotated_image_url: str = Field(
        ..., description="URL the frontend can fetch the annotated image from"
    )


class VideoDetectionResult(BaseModel):
    frame_count: int = Field(..., description="Number of frames processed")
    class_counts: Dict[str, int] = Field(
        ..., description="Summary count of detected objects per class across the video"
    )
    processing_time: float = Field(..., description="Total processing time in seconds")
    annotated_video_url: str = Field(
        ..., description="URL the frontend can fetch the annotated video from"
    )


class HealthStatus(BaseModel):
    status: str
    model_loaded: bool
