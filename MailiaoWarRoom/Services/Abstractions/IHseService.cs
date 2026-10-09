using MailiaoWarRoom.Models;

namespace MailiaoWarRoom.Services.Abstractions;

public interface IHseService
{
    Task<WeatherSnapshot> GetWeatherAsync(CancellationToken cancellationToken = default);
    Task<EmissionsSnapshot> GetEmissionsAsync(CancellationToken cancellationToken = default);
    Task<AlarmsSnapshot> GetAlarmsAsync(CancellationToken cancellationToken = default);
}
