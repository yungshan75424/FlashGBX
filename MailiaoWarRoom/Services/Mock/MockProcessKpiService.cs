using MailiaoWarRoom.Models;
using MailiaoWarRoom.Services.Abstractions;

namespace MailiaoWarRoom.Services.Mock;

public sealed class MockProcessKpiService : IProcessKpiService
{
    public Task<ProcessSummary> GetSummaryAsync(CancellationToken cancellationToken = default)
    {
        var summary = new ProcessSummary(
            OverallStatus: "注意",
            Cracker: new CrackerKpi(
                UnitName: "裂解一廠 OL-1",
                FeedRateTph: 186.4,
                FeedRateTargetTph: 200.0,
                LoadPercent: 93.2,
                Status: "正常"),
            Utilities: new UtilityKpi(
                PowerMw: 412.5,
                PowerCapacityMw: 480.0,
                SteamTph: 1280.0,
                SteamCapacityTph: 1500.0,
                IndustrialWaterM3h: 860.0,
                IndustrialWaterCapacityM3h: 1000.0),
            Production: new ProductionKpi(
                TodayOutputKt: 18.6,
                DailyTargetKt: 20.0,
                AchievementPercent: 93.0,
                Lines:
                [
                    new ProductLine("乙烯", 8.2, 8.8),
                    new ProductLine("丙烯", 4.6, 5.0),
                    new ProductLine("丁二烯", 1.4, 1.5),
                    new ProductLine("芳烴", 4.4, 4.7)
                ]),
            UpdatedAt: DateTimeOffset.Now);

        return Task.FromResult(summary);
    }

    public Task<ProcessTrends> GetTrendsAsync(string range = "24h", CancellationToken cancellationToken = default)
    {
        var hours = range.Equals("7d", StringComparison.OrdinalIgnoreCase) ? 7 * 24 : 24;
        var step = hours > 24 ? 6 : 1;
        var points = hours / step;
        var now = DateTimeOffset.Now;
        var timestamps = new List<string>(points);
        var feed = new List<double>(points);
        var load = new List<double>(points);
        var output = new List<double>(points);
        var power = new List<double>(points);

        var rng = new Random(42);
        for (var i = points - 1; i >= 0; i--)
        {
            var t = now.AddHours(-i * step);
            timestamps.Add(t.ToString("MM-dd HH:mm"));
            feed.Add(Math.Round(175 + rng.NextDouble() * 25, 1));
            load.Add(Math.Round(88 + rng.NextDouble() * 10, 1));
            output.Add(Math.Round(0.7 + rng.NextDouble() * 0.2, 2));
            power.Add(Math.Round(380 + rng.NextDouble() * 60, 1));
        }

        var trends = new ProcessTrends(
            Range: range,
            Timestamps: timestamps,
            Series:
            [
                new TrendSeries("裂解進料", "t/h", feed),
                new TrendSeries("運轉負荷", "%", load),
                new TrendSeries("小時產量", "kt", output),
                new TrendSeries("用電", "MW", power)
            ]);

        return Task.FromResult(trends);
    }
}
