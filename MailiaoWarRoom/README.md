# 六輕廠區戰情中心（Mailiao War Room Demo）

ASP.NET Core 8 MVC 後端 + Vue 3 / PrimeVue / ECharts 前端的廠區戰情儀表板 Demo。資料目前為 Mock，API 介面可日後替換真實 SCADA／OPC 來源。

## 需求

- [.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0)
- [Node.js 20+](https://nodejs.org/)（建置前端）

## 啟動步驟

```bash
cd MailiaoWarRoom/ClientApp
npm install
npm run build

cd ..
dotnet run --launch-profile http
```

瀏覽器開啟：<http://localhost:5062>

開發時可分開跑前端 Vite（API 代理到 5062）：

```bash
# 終端 1
cd MailiaoWarRoom && dotnet run --launch-profile http

# 終端 2
cd MailiaoWarRoom/ClientApp && npm run dev
```

## API（Mock）

| Method | Path | 說明 |
|--------|------|------|
| GET | `/api/process/summary` | 裂解／公用／產量摘要 |
| GET | `/api/process/trends?range=24h` | 產能趨勢序列 |
| GET | `/api/hse/weather` | 麥寮氣象 |
| GET | `/api/hse/emissions` | 排放測點 |
| GET | `/api/hse/alarms` | 警報列表 |

介面：`Services/Abstractions/`；Mock：`Services/Mock/`。替換 DI 註冊即可接真資料。

## 畫面

單頁深色工業戰情風：

1. **頂欄**：六輕廠區戰情中心、即時時鐘、綜合狀態 Tag
2. **製程 KPI**：裂解進料／負荷、公用（電／蒸汽／工業水）、產量 vs 目標、ECharts 趨勢
3. **安環戰情**：氣象 gauge、排放長條、警報 DataTable（可排序／篩選）
