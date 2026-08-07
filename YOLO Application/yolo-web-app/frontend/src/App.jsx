import { useState } from 'react'
import UploadForm from './components/UploadForm.jsx'
import ResultDisplay from './components/ResultDisplay.jsx'
import { detectImage, detectVideo } from './api.js'

export default function App() {
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [result, setResult] = useState(null)
  const [mediaType, setMediaType] = useState(null)

  async function handleSubmit(file, type, confidence) {
    setLoading(true)
    setError(null)
    setResult(null)
    setMediaType(type)

    try {
      const data = type === 'image'
        ? await detectImage(file, confidence)
        : await detectVideo(file, confidence)
      setResult(data)
    } catch (err) {
      setError(err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="app">
      <h1>YOLO Object Detection</h1>
      <UploadForm onSubmit={handleSubmit} loading={loading} />
      {loading && <p className="status">Đang xử lý, vui lòng đợi...</p>}
      {error && <p className="error">Lỗi: {error}</p>}
      <ResultDisplay result={result} mediaType={mediaType} />
    </div>
  )
}
