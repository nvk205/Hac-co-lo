import time
import uuid
from pathlib import Path
from typing import Dict


class Detector:
    def __init__(self, model_name: str = "yolo11n.pt"):
        from ultralytics import YOLO

        self.model = YOLO(model_name)

    def detect_image(self, image_path: str, output_dir: str, conf: float = 0.25) -> Dict:
        import cv2

        start = time.time()
        results = self.model.predict(source=image_path, conf=conf, verbose=False)
        result = results[0]

        detections = []
        for box in result.boxes:
            cls_id = int(box.cls[0])
            detections.append(
                {
                    "class_name": result.names[cls_id],
                    "confidence": float(box.conf[0]),
                    "bbox": [float(v) for v in box.xyxy[0].tolist()],
                }
            )

        annotated = result.plot()  # BGR numpy array with boxes drawn
        filename = f"{uuid.uuid4().hex}.jpg"
        output_path = str(Path(output_dir) / filename)
        cv2.imwrite(output_path, annotated)

        return {
            "detections": detections,
            "count": len(detections),
            "processing_time": time.time() - start,
            "annotated_image_url": f"/outputs/{filename}",
        }

    def detect_video(self, video_path: str, output_dir: str, conf: float = 0.25) -> Dict:
        import cv2

        start = time.time()
        cap = cv2.VideoCapture(video_path)
        fps = cap.get(cv2.CAP_PROP_FPS) or 25
        width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

        filename = f"{uuid.uuid4().hex}.mp4"
        output_path = str(Path(output_dir) / filename)
        fourcc = cv2.VideoWriter_fourcc(*"mp4v")
        writer = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

        class_counts: Dict[str, int] = {}
        frame_count = 0

        try:
            while True:
                ok, frame = cap.read()
                if not ok:
                    break
                frame_count += 1

                results = self.model.predict(source=frame, conf=conf, verbose=False)
                result = results[0]
                for box in result.boxes:
                    cls_id = int(box.cls[0])
                    name = result.names[cls_id]
                    class_counts[name] = class_counts.get(name, 0) + 1

                writer.write(result.plot())
        finally:
            cap.release()
            writer.release()

        return {
            "frame_count": frame_count,
            "class_counts": class_counts,
            "processing_time": time.time() - start,
            "annotated_video_url": f"/outputs/{filename}",
        }
