namespace MailiaoWarRoom.Models;

public record WeatherSnapshot(
    string Location,
    double TemperatureC,
    double HumidityPercent,
    double WindSpeedMs,
    double WindDirectionDeg,
    string WindDirectionLabel,
    double PressureHpa,
    string Condition,
    DateTimeOffset ObservedAt);

public record EmissionsSnapshot(
    DateTimeOffset UpdatedAt,
    IReadOnlyList<EmissionPoint> Points);

public record EmissionPoint(
    string Id,
    string Name,
    string Pollutant,
    double Value,
    double Limit,
    string Unit,
    string Status);

public record AlarmsSnapshot(
    DateTimeOffset UpdatedAt,
    IReadOnlyList<AlarmItem> Alarms);

public record AlarmItem(
    string Id,
    DateTimeOffset Time,
    string Severity,
    string Location,
    string Message,
    string Status);
