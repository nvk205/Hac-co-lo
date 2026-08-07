# YOLO Web Application

Backend (FastAPI + Ultralytics YOLO) và frontend (React + Vite) cho phép upload
ảnh/video, chạy object detection, và xem kết quả đã annotate.

```
browser -> frontend (React) -> backend API (FastAPI) -> YOLO model -> response -> frontend hiển thị
```

## Cấu trúc

```
backend/
  app/
    main.py          # FastAPI app, routes, CORS, startup model loading
    detection.py     # Wrapper quanh Ultralytics YOLO (ảnh + video)
    schemas.py        # Pydantic response models
  tests/
    test_api.py       # pytest: health, upload hợp lệ, file không hợp lệ, video
  .env.example
  pytest.ini
  requirements.txt
frontend/
  src/
    App.jsx
    api.js
    components/
      UploadForm.jsx
      ResultDisplay.jsx
  .env.example
  package.json
README.md
```

## Chạy local

### 1. Backend

```bash
cd backend
python -m venv .venv
.venv\Scripts\Activate.ps1
pip install -r requirements.txt

cp .env.example .env           # chỉnh sửa nếu cần (FRONTEND_ORIGIN, YOLO_MODEL...)

uvicorn app.main:app --reload --port 8000
```

Lần chạy đầu tiên, Ultralytics sẽ tự tải file trọng số `yolo11n.pt` . Server chạy tại `http://localhost:8000`, tài liệu API tự sinh tại `http://localhost:8000/docs`

Chạy test (không cần model thật, dùng `FakeDetector` nên chạy được offline):

```bash
cd backend
pytest -v
```

### 2. Frontend

```bash
cd "D:\2026_LAB_Training\YOLO Application\yolo-web-app\frontend"
npm install
copy .env.example .env
npm run dev
```

Mở `http://localhost:5173`.

## API endpoints

| Method | Endpoint | Mô tả |
|---     |---       |---|
| GET | `/health` | Health check, trả `{status, model_loaded}` |
| POST | `/api/detect/image` | Nhận `file` (multipart) + query `confidence` (0-1), trả detections + ảnh đã annotate |
| POST | `/api/detect/video` | Nhận `file` MP4 + query `confidence`, trả summary counts + video đã annotate |

Response mẫu cho `/api/detect/image`:

```json
{
  "detections": [
    {"class_name": "person", "confidence": 0.95, "bbox": [10.0, 10.0, 100.0, 100.0]}
  ],
  "count": 1,
  "processing_time": 0.03,
  "annotated_image_url": "/outputs/abc123.jpg"
}
```

## Định dạng file được hỗ trợ

- Ảnh: JPG, JPEG, PNG — tối đa 10MB
- Video: MP4 — tối đa 50MB

File sai định dạng hoặc quá dung lượng trả về lỗi `400` rõ ràng, không làm
crash server. Lỗi trong lúc inference trả về `500` kèm thông báo.

## Giới hạn (version đầu tiên)

- Không có database, không có login/authentication.
- Không custom train model — dùng model YOLO11 pretrained (mặc định `yolo11n.pt`).
- Video được xử lý đồng bộ trong request (không có queue cho video dài) — chỉ
  phù hợp với clip ngắn.
- File tạm và file output annotate được lưu trên đĩa cục bộ (`backend/outputs/`),
  chưa có cơ chế dọn dẹp tự động.

## Screenshots

_(thêm ảnh chụp màn hình giao diện upload và kết quả detect tại đây sau khi chạy thử)_

## Troubleshooting

- **`ModuleNotFoundError: No module named 'app'` khi chạy pytest** — chạy lệnh
  `pytest` từ đúng thư mục `backend/` (file `pytest.ini` đã cấu hình
  `pythonpath = .` cho trường hợp này).
- **CORS bị chặn trên trình duyệt** — kiểm tra `FRONTEND_ORIGIN` trong
  `backend/.env` khớp đúng với địa chỉ frontend đang chạy (mặc định
  `http://localhost:5173`).
- **Model tải chậm ở lần chạy đầu** — `yolo11n.pt` (~5MB) được Ultralytics tự
  tải về khi backend khởi động lần đầu; các lần sau dùng lại file đã cache.
- **`pip install` báo không tìm thấy `yolo11n.pt` / lỗi khi load model** —
  kiểm tra `ultralytics` đã ở bản `>=8.3.0` (`pip show ultralytics`); các bản
  cũ hơn chưa hỗ trợ YOLO11. Chạy `pip install -U ultralytics` để nâng cấp.
