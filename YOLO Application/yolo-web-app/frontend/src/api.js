export const API_BASE = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8000'

async function postFile(path, file, confidence) {
  const formData = new FormData()
  formData.append('file', file)

  const response = await fetch(`${API_BASE}${path}?confidence=${confidence}`, {
    method: 'POST',
    body: formData,
  })

  if (!response.ok) {
    const err = await response.json().catch(() => ({ detail: `Request failed (${response.status})` }))
    throw new Error(err.detail || `Request failed (${response.status})`)
  }

  return response.json()
}

export function detectImage(file, confidence) {
  return postFile('/api/detect/image', file, confidence)
}

export function detectVideo(file, confidence) {
  return postFile('/api/detect/video', file, confidence)
}
