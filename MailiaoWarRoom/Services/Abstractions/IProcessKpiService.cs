using MailiaoWarRoom.Models;

namespace MailiaoWarRoom.Services.Abstractions;

public interface IProcessKpiService
{
    Task<ProcessSummary> GetSummaryAsync(CancellationToken cancellationToken = default);
    Task<ProcessTrends> GetTrendsAsync(string range = "24h", CancellationToken cancellationToken = default);
}
