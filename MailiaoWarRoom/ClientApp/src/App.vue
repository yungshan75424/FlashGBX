<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { api } from '@/api/client'
import type {
  AlarmItem,
  EmissionsSnapshot,
  ProcessSummary,
  ProcessTrends,
  WeatherSnapshot,
} from '@/api/types'
import WarRoomHeader from '@/components/WarRoomHeader.vue'
import ProcessZone from '@/components/ProcessZone.vue'
import HseZone from '@/components/HseZone.vue'

const loading = ref(true)
const error = ref<string | null>(null)

const summary = ref<ProcessSummary | null>(null)
const trends = ref<ProcessTrends | null>(null)
const weather = ref<WeatherSnapshot | null>(null)
const emissions = ref<EmissionsSnapshot | null>(null)
const alarms = ref<AlarmItem[]>([])

onMounted(async () => {
  try {
    const [s, t, w, e, a] = await Promise.all([
      api.processSummary(),
      api.processTrends('24h'),
      api.weather(),
      api.emissions(),
      api.alarms(),
    ])
    summary.value = s
    trends.value = t
    weather.value = w
    emissions.value = e
    alarms.value = a.alarms
  } catch (err) {
    error.value = err instanceof Error ? err.message : '載入失敗'
  } finally {
    loading.value = false
  }
})
</script>

<template>
  <div class="war-room war-room-dark">
    <div v-if="loading" class="wr-loading">載入戰情資料中…</div>
    <div v-else-if="error" class="wr-error">{{ error }}</div>
    <template v-else-if="summary && trends && weather && emissions">
      <WarRoomHeader :overall-status="summary.overallStatus" />
      <div class="wr-grid">
        <ProcessZone :summary="summary" :trends="trends" />
        <HseZone :weather="weather" :emissions="emissions" :alarms="alarms" />
      </div>
    </template>
  </div>
</template>
