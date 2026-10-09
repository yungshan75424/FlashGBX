<script setup lang="ts">
import { computed } from 'vue'
import { use } from 'echarts/core'
import { CanvasRenderer } from 'echarts/renderers'
import { LineChart, BarChart } from 'echarts/charts'
import {
  GridComponent,
  LegendComponent,
  TooltipComponent,
} from 'echarts/components'
import VChart from 'vue-echarts'
import Tag from 'primevue/tag'
import type { ProcessSummary, ProcessTrends } from '@/api/types'

use([CanvasRenderer, LineChart, BarChart, GridComponent, LegendComponent, TooltipComponent])

const props = defineProps<{
  summary: ProcessSummary
  trends: ProcessTrends
}>()

const utilPowerPct = computed(() =>
  Math.round((props.summary.utilities.powerMw / props.summary.utilities.powerCapacityMw) * 100),
)
const utilSteamPct = computed(() =>
  Math.round((props.summary.utilities.steamTph / props.summary.utilities.steamCapacityTph) * 100),
)
const utilWaterPct = computed(() =>
  Math.round(
    (props.summary.utilities.industrialWaterM3h /
      props.summary.utilities.industrialWaterCapacityM3h) *
      100,
  ),
)

const trendOption = computed(() => {
  const feed = props.trends.series.find((s) => s.name === '裂解進料')
  const load = props.trends.series.find((s) => s.name === '運轉負荷')
  const power = props.trends.series.find((s) => s.name === '用電')

  return {
    backgroundColor: 'transparent',
    textStyle: { color: '#8fa3bd' },
    tooltip: { trigger: 'axis' },
    legend: {
      data: ['裂解進料', '運轉負荷', '用電'],
      textStyle: { color: '#8fa3bd' },
      top: 0,
    },
    grid: { left: 48, right: 48, top: 36, bottom: 28 },
    xAxis: {
      type: 'category',
      data: props.trends.timestamps,
      axisLine: { lineStyle: { color: '#2a3a52' } },
      axisLabel: { color: '#8fa3bd', fontSize: 10 },
    },
    yAxis: [
      {
        type: 'value',
        name: 't/h · %',
        nameTextStyle: { color: '#8fa3bd' },
        splitLine: { lineStyle: { color: 'rgba(94,234,212,0.08)' } },
        axisLabel: { color: '#8fa3bd' },
      },
      {
        type: 'value',
        name: 'MW',
        nameTextStyle: { color: '#8fa3bd' },
        splitLine: { show: false },
        axisLabel: { color: '#8fa3bd' },
      },
    ],
    series: [
      {
        name: '裂解進料',
        type: 'line',
        smooth: true,
        showSymbol: false,
        data: feed?.values ?? [],
        lineStyle: { width: 2, color: '#2dd4bf' },
        itemStyle: { color: '#2dd4bf' },
        areaStyle: {
          color: {
            type: 'linear',
            x: 0,
            y: 0,
            x2: 0,
            y2: 1,
            colorStops: [
              { offset: 0, color: 'rgba(45,212,191,0.25)' },
              { offset: 1, color: 'rgba(45,212,191,0)' },
            ],
          },
        },
      },
      {
        name: '運轉負荷',
        type: 'line',
        smooth: true,
        showSymbol: false,
        data: load?.values ?? [],
        lineStyle: { width: 2, color: '#38bdf8' },
        itemStyle: { color: '#38bdf8' },
      },
      {
        name: '用電',
        type: 'line',
        yAxisIndex: 1,
        smooth: true,
        showSymbol: false,
        data: power?.values ?? [],
        lineStyle: { width: 2, color: '#f59e0b' },
        itemStyle: { color: '#f59e0b' },
      },
    ],
  }
})

const productionBarOption = computed(() => ({
  backgroundColor: 'transparent',
  textStyle: { color: '#8fa3bd' },
  tooltip: { trigger: 'axis' },
  legend: {
    data: ['當日產量', '目標'],
    textStyle: { color: '#8fa3bd' },
    top: 0,
  },
  grid: { left: 48, right: 16, top: 36, bottom: 28 },
  xAxis: {
    type: 'category',
    data: props.summary.production.lines.map((l) => l.name),
    axisLine: { lineStyle: { color: '#2a3a52' } },
    axisLabel: { color: '#8fa3bd' },
  },
  yAxis: {
    type: 'value',
    name: 'kt',
    nameTextStyle: { color: '#8fa3bd' },
    splitLine: { lineStyle: { color: 'rgba(94,234,212,0.08)' } },
    axisLabel: { color: '#8fa3bd' },
  },
  series: [
    {
      name: '當日產量',
      type: 'bar',
      barWidth: 18,
      data: props.summary.production.lines.map((l) => l.outputKt),
      itemStyle: { color: '#2dd4bf', borderRadius: [2, 2, 0, 0] },
    },
    {
      name: '目標',
      type: 'bar',
      barWidth: 18,
      data: props.summary.production.lines.map((l) => l.targetKt),
      itemStyle: { color: 'rgba(143,163,189,0.35)', borderRadius: [2, 2, 0, 0] },
    },
  ],
}))

function statusSeverity(status: string) {
  if (status === '正常') return 'success'
  if (status === '注意') return 'warn'
  return 'danger'
}
</script>

<template>
  <section class="wr-zone">
    <div class="wr-panel">
      <div class="wr-panel-title">
        <h2>製程／產能 KPI</h2>
        <Tag :severity="statusSeverity(summary.cracker.status)" :value="summary.cracker.unitName" />
      </div>
      <div class="wr-kpi-row">
        <div class="wr-kpi">
          <span class="label">裂解進料</span>
          <span class="value">
            {{ summary.cracker.feedRateTph.toFixed(1) }}
            <span class="unit">t/h</span>
          </span>
          <span class="meta">目標 {{ summary.cracker.feedRateTargetTph }} t/h</span>
        </div>
        <div class="wr-kpi">
          <span class="label">運轉負荷</span>
          <span class="value">
            {{ summary.cracker.loadPercent.toFixed(1) }}
            <span class="unit">%</span>
          </span>
          <span class="meta">狀態 {{ summary.cracker.status }}</span>
        </div>
        <div class="wr-kpi">
          <span class="label">當日產量達成</span>
          <span class="value">
            {{ summary.production.achievementPercent.toFixed(0) }}
            <span class="unit">%</span>
          </span>
          <span class="meta">
            {{ summary.production.todayOutputKt }} / {{ summary.production.dailyTargetKt }} kt
          </span>
        </div>
      </div>
      <div class="wr-kpi-row" style="margin-top: 0.65rem">
        <div class="wr-kpi">
          <span class="label">公用電力</span>
          <span class="value">
            {{ summary.utilities.powerMw.toFixed(1) }}
            <span class="unit">MW</span>
          </span>
          <span class="meta">負載 {{ utilPowerPct }}% · 容量 {{ summary.utilities.powerCapacityMw }} MW</span>
        </div>
        <div class="wr-kpi">
          <span class="label">蒸汽</span>
          <span class="value">
            {{ summary.utilities.steamTph.toFixed(0) }}
            <span class="unit">t/h</span>
          </span>
          <span class="meta">負載 {{ utilSteamPct }}% · 容量 {{ summary.utilities.steamCapacityTph }} t/h</span>
        </div>
        <div class="wr-kpi">
          <span class="label">工業水</span>
          <span class="value">
            {{ summary.utilities.industrialWaterM3h.toFixed(0) }}
            <span class="unit">m³/h</span>
          </span>
          <span class="meta">
            負載 {{ utilWaterPct }}% · 容量 {{ summary.utilities.industrialWaterCapacityM3h }} m³/h
          </span>
        </div>
      </div>
    </div>

    <div class="wr-panel">
      <div class="wr-panel-title">
        <h2>產能趨勢（{{ trends.range }}）</h2>
      </div>
      <VChart class="wr-chart wr-chart-tall" :option="trendOption" autoresize />
    </div>

    <div class="wr-panel">
      <div class="wr-panel-title">
        <h2>產品線產量 vs 目標</h2>
      </div>
      <VChart class="wr-chart" :option="productionBarOption" autoresize />
    </div>
  </section>
</template>
