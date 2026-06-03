#pragma once

#include <chrono>
#include <cmath>
#include <string>

namespace helm::telemetry {

using SteadyClock = std::chrono::steady_clock;
using SteadyTime = SteadyClock::time_point;

enum class SensorQuality {
    Missing,
    Stale,
    Good,
    Warning,
    Alarm
};

enum class AnchorStatus {
    Disarmed,
    HoldingOk,
    NearLimit,
    DragWarning,
    DragAlarm,
    GpsLost
};

enum class BilgeStatus {
    Dry,
    PumpRunning,
    HighWater,
    Fault
};

enum class WindlassStatus {
    Locked,
    Armed,
    RunningUp,
    RunningDown,
    Fault
};

template <typename T>
struct TimedValue {
    T value{};
    SteadyTime updated{};
    SensorQuality quality{SensorQuality::Missing};
    std::string source{"dummy"};

    [[nodiscard]] bool has_value() const noexcept {
        return quality != SensorQuality::Missing;
    }

    [[nodiscard]] bool is_stale(SteadyTime now, std::chrono::milliseconds max_age) const noexcept {
        if (!has_value()) {
            return true;
        }
        return (now - updated) > max_age;
    }
};

struct GeoPoint {
    double latitude_deg{0.0};
    double longitude_deg{0.0};
};

struct NavigationState {
    TimedValue<double> latitude_deg;
    TimedValue<double> longitude_deg;
    TimedValue<double> sog_kt;
    TimedValue<double> cog_deg;
    TimedValue<double> heading_true_deg;
    TimedValue<double> depth_m;
};

struct WindState {
    TimedValue<double> awa_deg;
    TimedValue<double> aws_kt;
    TimedValue<double> twa_deg;
    TimedValue<double> tws_kt;
};

struct AnchorState {
    AnchorStatus status{AnchorStatus::Disarmed};
    GeoPoint anchor_position{};
    TimedValue<double> distance_m;
    TimedValue<double> radius_m;
};

struct AisSummary {
    TimedValue<int> targets;
    TimedValue<int> danger_targets;
    TimedValue<double> nearest_range_nm;
    TimedValue<double> threat_cpa_nm;
    TimedValue<double> threat_tcpa_min;
};

struct BilgeState {
    BilgeStatus status{BilgeStatus::Dry};
    TimedValue<bool> high_water;
    TimedValue<bool> pump_on;
    TimedValue<double> pump_current_a;
    TimedValue<double> runtime_today_min;
};

struct WindlassState {
    WindlassStatus status{WindlassStatus::Locked};
    TimedValue<double> chain_deployed_m;
    TimedValue<double> motor_current_a;
    TimedValue<bool> breaker_on;
};

struct ElectricalState {
    TimedValue<double> house_voltage_v;
    TimedValue<double> house_current_a;
};

struct TelemetrySnapshot {
    SteadyTime timestamp{};
    NavigationState nav;
    WindState wind;
    AnchorState anchor;
    AisSummary ais;
    BilgeState bilge;
    WindlassState windlass;
    ElectricalState electrical;
};

} // namespace helm::telemetry
