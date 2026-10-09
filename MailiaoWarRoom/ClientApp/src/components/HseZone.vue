<script setup lang="ts">
import { computed, ref } from 'vue'
import { use } from 'echarts/core'
import { CanvasRenderer } from 'echarts/renderers'
import { BarChart, GaugeChart } from 'echarts/charts'
import {
  GridComponent,
  LegendComponent,
  TooltipComponent,
} from 'echarts/components'
import VChart from 'vue-echarts'
import DataTable from 'primevue/datatable'
import Column from 'primevue/column'
import Tag from 'primevue/tag'
import Select from 'primevue/select'
import type { AlarmItem, EmissionsSnapshot, WeatherSnapshot } from '@/api/types'

use([CanvasRenderer, BarChart, GaugeChart, GridComponent, LegendComponent, TooltipComponent])

const props = defineProps<{
  weather: WeatherSnapshot
  emissions: EmissionsSnapshot
  alarms: AlarmItem[]
}>()

const severityFilter = ref<string | null>(null)
const severityOptions = [
  { label: '全部等級', value: null },
  { label: '嚴重', value: '嚴重' },
  { label: '警告', value: '警告' },
  { label: '注意', value: '注意' },
  { label: '資訊', value: '資訊' },
]

const filteredAlarms = computed(() => {
  if (!severityFilter.value) return props.alarms
  return props.alarms.filter((a) => a.severity === severityFilter.value)
})

const windGaugeOption = computed(() => ({
  backgroundColor: 'transparent',
  series: [
    {
      type: 'gauge',
      min: 0,
      max: 20,
      splitNumber: 4,
      radius: '90%',
      axisLine: {
        lineStyle: {
          width: 12,
          color: [
            [0.4, '#34d399'],
            [0.7, '#f59e0b'],
            [1, '#ef4444'],
          ],
        },
      },
      pointer: { itemStyle: { color: '#2dd4bf' }, width: 4 },
      axisTick: { distance: -12, length: 6, lineStyle: { color: '#8fa3bd' } },
      splitLine: { distance: -16, length: 12, lineStyle: { color: '#8fa3bd' } },
      axisLabel: { color: '#8fa3bd', distance: 18, fontSize: 10 },
      detail: {
        valueAnimation: true,
        formatter: '{value} m/s',
        color: '#e8eef7',
        fontSize: 14,
        offsetCenter: [0, '70%'],
      },
      title: {
        offsetCenter: [0, '95%'],
        color: '#8fa3bd',
        fontSize: 11,
      },
      data: [{ value: props.weather.windSpeedMs, name: props.weather.windDirectionLabel }],
    },
  ],
}))

const emissionsBarOption = computed(() => ({
  backgroundColor: 'transparent',
  textStyle: { color: '#8fa3bd' },
  tooltip: {
    trigger: 'axis',
    formatter: (params: { name: string; value: number; dataIndex: number }[]) => {
      const p = params[0]
      const point = props.emissions.points[p.dataIndex]
      return `${point.name}<br/>${point.pollutant}: ${point.value} / ${point.limit} ${point.unit}`
    },
  },
  grid: { left: 100, right: 24, top: 16, bottom: 28 },
  xAxis: {
    type: 'value',
    max: 100,
    axisLabel: { color: '#8fa3bd', formatter: '{value}%' },
    splitLine: { lineStyle: { color: 'rgba(94,234,212,0.08)' } },
  },
  yAxis: {
    type: 'category',
    data: props.emissions.points.map((p) => `${p.name}·${p.pollutant}`),
    axisLabel: { color: '#8fa3bd', fontSize: 10 },
    axisLine: { lineStyle: { color: '#2a3a52' } },
  },
  series: [
    {
      type: 'bar',
      data: props.emissions.points.map((p) => {
        const pct = Math.round((p.value / p.limit) * 100)
        let color = '#34d399'
        if (pct >= 85) color = '#ef4444'
        else if (pct >= 70) color = '#f59e0b'
        return { value: pct, itemStyle: { color, borderRadius: [0, 2, 2, 0] } }
      }),
      barWidth: 12,
      label: {
        show: true,
        position: 'right',
        color: '#8fa3bd',
        fontSize: 10,
        formatter: '{c}%',
      },
    },
  ],
}))

function severitySeverity(s: string) {
  switch (s) {
    case '嚴重':
      return 'danger'
    case '警告':
      return 'warn'
    case '注意':
      return 'info'
    default:
      return 'secondary'
  }
}

function formatTime(iso: string) {
  return new Date(iso).toLocaleString('zh-TW', {
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false,
  })
}
</script>

<template>
  <section class="wr-zone">
    <div class="wr-panel">
      <div class="wr-panel-title">
        <h2>安環戰情 · 麥寮氣象</h2>
        <Tag severity="info" :value="weather.condition" />
      </div>
      <div class="wr-weather-grid">
        <div class="wr-weather-stats">
          <div class="wr-stat">
            <div class="label">溫度</div>
            <div class="value">{{ weather.temperatureC.toFixed(1) }} °C</div>
          </div>
          <div class="wr-stat">
            <div class="label">相對濕度</div>
            <div class="value">{{ weather.humidityPercent }} %</div>
          </div>
          <div class="wr-stat">
            <div class="label">氣壓</div>
            <div class="value">{{ weather.pressureHpa.toFixed(1) }} hPa</div>
          </div>
          <div class="wr-stat">
            <div class="label">測站</div>
            <div class="value" style="font-size: 0.9rem">{{ weather.location }}</div>
          </div>
        </div>
        <VChart class="wr-chart" style="height: 180px" :option="windGaugeOption" autoresize />
      </div>
    </div>

    <div class="wr-panel">
      <div class="wr-panel-title">
        <h2>重點排放測點（佔管制上限 %）</h2>
      </div>
      <VChart class="wr-chart wr-chart-tall" :option="emissionsBarOption" autoresize />
    </div>

    <div class="wr-panel" style="flex: 1">
      <div class="wr-panel-title">
        <h2>即時警報清單</h2>
        <Select
          v-model="severityFilter"
          :options="severityOptions"
          option-label="label"
          option-value="value"
          placeholder="篩選等級"
          style="min-width: 9rem"
        />
      </div>
      <DataTable
        :value="filteredAlarms"
        sort-field="time"
        :sort-order="-1"
        size="small"
        striped-rows
        paginator
        :rows="5"
        :rows-per-page-options="[5, 10]"
        table-style="min-width: 100%"
      >
        <Column field="time" header="時間" sortable style="min-width: 7.5rem">
          <template #body="{ data }">
            {{ formatTime(data.time) }}
          </template>
        </Column>
        <Column field="severity" header="等級" sortable style="min-width: 5rem">
          <template #body="{ data }">
            <Tag :severity="severitySeverity(data.severity)" :value="data.severity" />
          </template>
        </Column>
        <Column field="location" header="位置" sortable style="min-width: 6rem" />
        <Column field="message" header="訊息" style="min-width: 12rem" />
        <Column field="status" header="狀態" sortable style="min-width: 5rem" />
      </DataTable>
    </div>
  </section>
</template>
