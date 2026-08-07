import { API_BASE } from '../api.js'

export default function ResultDisplay({ result, mediaType }) {
  if (!result) return null

  if (mediaType === 'image') {
    return (
      <div className="result">
        <h3>Kết quả</h3>
        <img src={`${API_BASE}${result.annotated_image_url}`} alt="annotated result" />
        <p>Số object phát hiện: {result.count}</p>
        <p>Thời gian xử lý: {result.processing_time.toFixed(2)}s</p>
        <ul>
          {result.detections.map((d, i) => (
            <li key={i}>
              {d.class_name} — {(d.confidence * 100).toFixed(1)}%
            </li>
          ))}
        </ul>
      </div>
    )
  }

  return (
    <div className="result">
      <h3>Kết quả</h3>
      <video src={`${API_BASE}${result.annotated_video_url}`} controls />
      <p>Số frame đã xử lý: {result.frame_count}</p>
      <p>Thời gian xử lý: {result.processing_time.toFixed(2)}s</p>
      <ul>
        {Object.entries(result.class_counts).map(([name, count]) => (
          <li key={name}>
            {name}: {count}
          </li>
        ))}
      </ul>
      <a href={`${API_BASE}${result.annotated_video_url}`} download>
        Tải video xuống
      </a>
    </div>
  )
}
