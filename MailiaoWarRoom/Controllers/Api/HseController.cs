using MailiaoWarRoom.Services.Abstractions;
using Microsoft.AspNetCore.Mvc;

namespace MailiaoWarRoom.Controllers.Api;

[ApiController]
[Route("api/hse")]
public sealed class HseController : ControllerBase
{
    private readonly IHseService _hseService;

    public HseController(IHseService hseService)
    {
        _hseService = hseService;
    }

    [HttpGet("weather")]
    public async Task<IActionResult> GetWeather(CancellationToken cancellationToken)
    {
        var weather = await _hseService.GetWeatherAsync(cancellationToken);
        return Ok(weather);
    }

    [HttpGet("emissions")]
    public async Task<IActionResult> GetEmissions(CancellationToken cancellationToken)
    {
        var emissions = await _hseService.GetEmissionsAsync(cancellationToken);
        return Ok(emissions);
    }

    [HttpGet("alarms")]
    public async Task<IActionResult> GetAlarms(CancellationToken cancellationToken)
    {
        var alarms = await _hseService.GetAlarmsAsync(cancellationToken);
        return Ok(alarms);
    }
}
