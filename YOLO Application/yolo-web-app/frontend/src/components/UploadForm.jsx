import { useState } from 'react'

const ACCEPTED_IMAGE_TYPES = ['image/jpeg', 'image/jpg', 'image/png']
const ACCEPTED_VIDEO_TYPES = ['video/mp4']

export default function UploadForm({ onSubmit, loading }) {
  const [file, setFile] = useState(null)
  const [previewUrl, setPreviewUrl] = useState(null)
  const [mediaType, setMediaType] = useState(null)
  const [confidence, setConfidence] = useState(0.25)
  const [localError, setLocalError] = useState(null)

  function handleFileChange(e) {
    const selected = e.target.files[0]
    setLocalError(null)
    if (!selected) return

    if (ACCEPTED_IMAGE_TYPES.includes(selected.type)) {
      setMediaType('image')
    } else if (ACCEPTED_VIDEO_TYPES.includes(selected.type)) {
      setMediaType('video')
    } else {
      setLocalError('File không được hỗ trợ. Chỉ nhận JPG, JPEG, PNG hoặc MP4.')
      setFile(null)
      setPreviewUrl(null)
      return
    }

    setFile(selected)
    setPreviewUrl(URL.createObjectURL(selected))
  }

  function handleSubmit(e) {
    e.preventDefault()
    if (!file) {
      setLocalError('Vui lòng chọn một file trước.')
      return
    }
    onSubmit(file, mediaType, confidence)
  }

  return (
    <form onSubmit={handleSubmit} className="upload-form">
      <label className="field">
        <span>Chọn ảnh (JPG/PNG) hoặc video (MP4)</span>
        <input type="file" accept=".jpg,.jpeg,.png,.mp4" onChange={handleFileChange} />
      </label>

      <label className="field">
        <span>Ngưỡng confidence: {confidence.toFixed(2)}</span>
        <input
          type="range"
          min="0.05"
          max="0.95"
          step="0.05"
          value={confidence}
          onChange={(e) => setConfidence(parseFloat(e.target.value))}
        />
      </label>

      {previewUrl && mediaType === 'image' && (
        <img src={previewUrl} alt="preview" className="preview" />
      )}
      {previewUrl && mediaType === 'video' && (
        <video src={previewUrl} controls className="preview" />
      )}

      {localError && <p className="error">{localError}</p>}

      <button type="submit" disabled={loading}>
        {loading ? 'Đang xử lý...' : 'Phát hiện đối tượng'}
      </button>
    </form>
  )
}
