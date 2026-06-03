#include "telemetry/DummyTelemetrySource.h"

#include <cmath>

namespace helm::telemetry {
namespace {

template <typename T>
TimedValue<T> make_value(T value, SteadyTime now, SensorQuality q = SensorQuality::Good) {
    TimedValue<T> tv;
    tv.value = value;
    tv.updated = now;
    tv.quality = q;
    tv.source = "dummy";
    return tv;
}

double wrap360(double deg) {
    while (deg < 0.0) deg += 360.0;
    while (deg >= 360.0) deg -= 360.0;
    return deg;
}

} // namespace

TelemetrySnapshot DummyTelemetrySource::next_snapshot() {
    const auto now = SteadyClock::now();
    const auto elapsed = std::chrono::duration<double>(now - start_).count();

    TelemetrySnapshot s;
    s.timestamp = now;

    const double heading = wrap360(247.0 + 8.0 * std::sin(elapsed * 0.17));
    const double cog = wrap360(250.0 + 4.0 * std::sin(elapsed * 0.11));
    const double sog = 6.8 + 0.3 * std::sin(elapsed * 0.40);
    const double depth = 18.2 + 0.8 * std::sin(elapsed * 0.05);

    s.nav.latitude_deg = make_value(40.7167 + 0.0001 * std::sin(elapsed * 0.05), now);
    s.nav.longitude_deg = make_value(-74.0167 + 0.0001 * std::cos(elapsed * 0.05), now);
    s.nav.heading_true_deg = make_value(heading, now);
    s.nav.cog_deg = make_value(cog, now);
    s.nav.sog_kt = make_value(sog, now);
    s.nav.depth_m = make_value(depth, now);

    s.wind.awa_deg = make_value(-35.0 + 5.0 * std::sin(elapsed * 0.31), now);
    s.wind.aws_kt = make_value(18.0 + 2.0 * std::sin(elapsed * 0.23), now);
    s.wind.twa_deg = make_value(-42.0 + 4.0 * std::sin(elapsed * 0.27), now);
    s.wind.tws_kt = make_value(14.8 + 1.5 * std::sin(elapsed * 0.19), now);

    s.anchor.status = AnchorStatus::HoldingOk;
    s.anchor.distance_m = make_value(23.0 + 5.0 * std::sin(elapsed * 0.10), now);
    s.anchor.radius_m = make_value(50.0, now);

    s.ais.targets = make_value(12, now);
    s.ais.danger_targets = make_value((std::sin(elapsed * 0.08) > 0.85) ? 1 : 0, now,
                                      (std::sin(elapsed * 0.08) > 0.85) ? SensorQuality::Alarm : SensorQuality::Good);
    s.ais.nearest_range_nm = make_value(0.82 + 0.1 * std::sin(elapsed * 0.18), now);
    s.ais.threat_cpa_nm = make_value(0.42 + 0.1 * std::sin(elapsed * 0.13), now);
    s.ais.threat_tcpa_min = make_value(14.0 + 3.0 * std::sin(elapsed * 0.21), now);

    const bool pump_on = std::sin(elapsed * 0.06) > 0.93;
    s.bilge.status = pump_on ? BilgeStatus::PumpRunning : BilgeStatus::Dry;
    s.bilge.high_water = make_value(false, now);
    s.bilge.pump_on = make_value(pump_on, now);
    s.bilge.pump_current_a = make_value(pump_on ? 4.8 : 0.0, now);
    s.bilge.runtime_today_min = make_value(3.0 + 0.2 * std::sin(elapsed * 0.03), now);

    s.windlass.status = WindlassStatus::Locked;
    s.windlass.chain_deployed_m = make_value(42.0, now);
    s.windlass.motor_current_a = make_value(0.0, now);
    s.windlass.breaker_on = make_value(true, now);

    s.electrical.house_voltage_v = make_value(12.7 + 0.1 * std::sin(elapsed * 0.07), now);
    s.electrical.house_current_a = make_value(-8.0 + 1.0 * std::sin(elapsed * 0.14), now);

    return s;
}

} // namespace helm::telemetry
