using MailiaoWarRoom.Services.Abstractions;
using Microsoft.AspNetCore.Mvc;

namespace MailiaoWarRoom.Controllers.Api;

[ApiController]
[Route("api/process")]
public sealed class ProcessController : ControllerBase
{
    private readonly IProcessKpiService _processKpiService;

    public ProcessController(IProcessKpiService processKpiService)
    {
        _processKpiService = processKpiService;
    }

    [HttpGet("summary")]
    public async Task<IActionResult> GetSummary(CancellationToken cancellationToken)
    {
        var summary = await _processKpiService.GetSummaryAsync(cancellationToken);
        return Ok(summary);
    }

    [HttpGet("trends")]
    public async Task<IActionResult> GetTrends([FromQuery] string range = "24h", CancellationToken cancellationToken = default)
    {
        var trends = await _processKpiService.GetTrendsAsync(range, cancellationToken);
        return Ok(trends);
    }
}
