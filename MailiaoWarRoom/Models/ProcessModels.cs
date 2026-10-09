namespace MailiaoWarRoom.Models;

public record ProcessSummary(
    string OverallStatus,
    CrackerKpi Cracker,
    UtilityKpi Utilities,
    ProductionKpi Production,
    DateTimeOffset UpdatedAt);

public record CrackerKpi(
    string UnitName,
    double FeedRateTph,
    double FeedRateTargetTph,
    double LoadPercent,
    string Status);

public record UtilityKpi(
    double PowerMw,
    double PowerCapacityMw,
    double SteamTph,
    double SteamCapacityTph,
    double IndustrialWaterM3h,
    double IndustrialWaterCapacityM3h);

public record ProductionKpi(
    double TodayOutputKt,
    double DailyTargetKt,
    double AchievementPercent,
    IReadOnlyList<ProductLine> Lines);

public record ProductLine(string Name, double OutputKt, double TargetKt);

public record ProcessTrends(
    string Range,
    IReadOnlyList<string> Timestamps,
    IReadOnlyList<TrendSeries> Series);

public record TrendSeries(string Name, string Unit, IReadOnlyList<double> Values);
