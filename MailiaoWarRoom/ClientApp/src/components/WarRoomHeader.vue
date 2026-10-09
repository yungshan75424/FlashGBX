<script setup lang="ts">
import { computed, onMounted, onUnmounted, ref } from 'vue'
import Tag from 'primevue/tag'

const props = defineProps<{
  overallStatus: string
}>()

const now = ref(new Date())
let timer: ReturnType<typeof setInterval> | undefined

onMounted(() => {
  timer = setInterval(() => {
    now.value = new Date()
  }, 1000)
})

onUnmounted(() => {
  if (timer) clearInterval(timer)
})

const clockText = computed(() =>
  now.value.toLocaleString('zh-TW', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: false,
  }),
)

const statusSeverity = computed(() => {
  switch (props.overallStatus) {
    case '正常':
      return 'success'
    case '注意':
      return 'warn'
    case '異常':
      return 'danger'
    default:
      return 'info'
  }
})
</script>

<template>
  <header class="wr-header">
    <div class="wr-brand">
      <h1>六輕廠區戰情中心</h1>
      <span class="subtitle">Mailiao Process &amp; HSE War Room · Demo</span>
    </div>
    <div class="wr-header-meta">
      <div class="wr-clock">{{ clockText }}</div>
      <Tag
        class="wr-status-pulse"
        :severity="statusSeverity"
        :value="`綜合狀態：${overallStatus}`"
        rounded
      />
    </div>
  </header>
</template>
