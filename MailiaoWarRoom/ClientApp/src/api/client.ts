import type {
  AlarmsSnapshot,
  EmissionsSnapshot,
  ProcessSummary,
  ProcessTrends,
  WeatherSnapshot,
} from './types'

const API_BASE = import.meta.env.VITE_API_BASE ?? ''

async function getJson<T>(path: string): Promise<T> {
  const response = await fetch(`${API_BASE}${path}`)
  if (!response.ok) {
    throw new Error(`API ${path} failed: ${response.status}`)
  }
  return response.json() as Promise<T>
}

export const api = {
  processSummary: () => getJson<ProcessSummary>('/api/process/summary'),
  processTrends: (range = '24h') =>
    getJson<ProcessTrends>(`/api/process/trends?range=${encodeURIComponent(range)}`),
  weather: () => getJson<WeatherSnapshot>('/api/hse/weather'),
  emissions: () => getJson<EmissionsSnapshot>('/api/hse/emissions'),
  alarms: () => getJson<AlarmsSnapshot>('/api/hse/alarms'),
}
