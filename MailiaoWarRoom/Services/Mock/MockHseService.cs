using MailiaoWarRoom.Models;
using MailiaoWarRoom.Services.Abstractions;

namespace MailiaoWarRoom.Services.Mock;

public sealed class MockHseService : IHseService
{
    public Task<WeatherSnapshot> GetWeatherAsync(CancellationToken cancellationToken = default)
    {
        var weather = new WeatherSnapshot(
            Location: "雲林麥寮廠區",
            TemperatureC: 28.6,
            HumidityPercent: 72,
            WindSpeedMs: 4.8,
            WindDirectionDeg: 225,
            WindDirectionLabel: "西南風",
            PressureHpa: 1008.2,
            Condition: "多雲",
            ObservedAt: DateTimeOffset.Now);

        return Task.FromResult(weather);
    }

    public Task<EmissionsSnapshot> GetEmissionsAsync(CancellationToken cancellationToken = default)
    {
        var snapshot = new EmissionsSnapshot(
            UpdatedAt: DateTimeOffset.Now,
            Points:
            [
                new EmissionPoint("EM-01", "裂解煙囪 A", "NOx", 42.5, 70.0, "ppm", "正常"),
                new EmissionPoint("EM-02", "裂解煙囪 A", "SOx", 18.2, 50.0, "ppm", "正常"),
                new EmissionPoint("EM-03", "公用鍋爐 B", "NOx", 58.0, 70.0, "ppm", "注意"),
                new EmissionPoint("EM-04", "公用鍋爐 B", "CO", 12.4, 30.0, "ppm", "正常"),
                new EmissionPoint("EM-05", "芳烴區煙囪 C", "VOC", 8.6, 15.0, "ppm", "正常"),
                new EmissionPoint("EM-06", "碼頭區邊界", "PM10", 48.0, 100.0, "µg/m³", "正常")
            ]);

        return Task.FromResult(snapshot);
    }

    public Task<AlarmsSnapshot> GetAlarmsAsync(CancellationToken cancellationToken = default)
    {
        var now = DateTimeOffset.Now;
        var snapshot = new AlarmsSnapshot(
            UpdatedAt: now,
            Alarms:
            [
                new AlarmItem("AL-2401", now.AddMinutes(-12), "警告", "公用鍋爐 B", "NOx 排放接近管制上限（58/70 ppm）", "開放"),
                new AlarmItem("AL-2402", now.AddMinutes(-35), "注意", "裂解一廠 OL-1", "進料流量偏離目標約 6.8%", "確認中"),
                new AlarmItem("AL-2403", now.AddHours(-1.5), "資訊", "工業水系統", "冷卻水塔 #3 補水率偏高", "開放"),
                new AlarmItem("AL-2404", now.AddHours(-3), "警告", "芳烴區", "VOC 瞬間峰值（已回落）", "已關閉"),
                new AlarmItem("AL-2405", now.AddHours(-5), "嚴重", "蒸汽管網", "高壓蒸汽壓力短暫低於 38 kg/cm²", "已關閉"),
                new AlarmItem("AL-2406", now.AddHours(-8), "注意", "碼頭區", "風速 > 8 m/s 作業注意通報", "已關閉")
            ]);

        return Task.FromResult(snapshot);
    }
}
