export interface ProcessSummary {
  overallStatus: string
  cracker: {
    unitName: string
    feedRateTph: number
    feedRateTargetTph: number
    loadPercent: number
    status: string
  }
  utilities: {
    powerMw: number
    powerCapacityMw: number
    steamTph: number
    steamCapacityTph: number
    industrialWaterM3h: number
    industrialWaterCapacityM3h: number
  }
  production: {
    todayOutputKt: number
    dailyTargetKt: number
    achievementPercent: number
    lines: { name: string; outputKt: number; targetKt: number }[]
  }
  updatedAt: string
}

export interface ProcessTrends {
  range: string
  timestamps: string[]
  series: { name: string; unit: string; values: number[] }[]
}

export interface WeatherSnapshot {
  location: string
  temperatureC: number
  humidityPercent: number
  windSpeedMs: number
  windDirectionDeg: number
  windDirectionLabel: string
  pressureHpa: number
  condition: string
  observedAt: string
}

export interface EmissionPoint {
  id: string
  name: string
  pollutant: string
  value: number
  limit: number
  unit: string
  status: string
}

export interface EmissionsSnapshot {
  updatedAt: string
  points: EmissionPoint[]
}

export interface AlarmItem {
  id: string
  time: string
  severity: string
  location: string
  message: string
  status: string
}

export interface AlarmsSnapshot {
  updatedAt: string
  alarms: AlarmItem[]
}
