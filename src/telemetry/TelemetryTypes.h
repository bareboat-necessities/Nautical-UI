#pragma once

#include <chrono>
#include <cmath>
#include <cstdint>
#include <string>

namespace helm::telemetry {

using SteadyClock = std::chrono::steady_clock;
using SteadyTime = SteadyClock::time_point;

inline constexpr auto DefaultSensorStaleAfter = std::chrono::milliseconds(2500);
inline constexpr auto GpsStaleAfter = std::chrono::milliseconds(5000);
inline constexpr auto AisStaleAfter = std::chrono::milliseconds(15000);
inline constexpr auto SystemsStaleAfter = std::chrono::milliseconds(5000);

enum class SensorQuality {
    Missing,
    Stale,
    Good,
    Warning,
    Alarm
};

enum class LinkStatus {
    Disabled,
    Connecting,
    Live,
    Stale,
    Error
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
    std::string source{"unset"};

    [[nodiscard]] bool has_value() const noexcept {
        return quality != SensorQuality::Missing;
    }

    [[nodiscard]] bool is_stale(SteadyTime now, std::chrono::milliseconds max_age) const noexcept {
        if (!has_value()) {
            return true;
        }
        return (now - updated) > max_age;
    }

    void set(T new_value, SteadyTime when, std::string src, SensorQuality q = SensorQuality::Good) {
        value = new_value;
        updated = when;
        quality = q;
        source = std::move(src);
    }

    void mark_stale() noexcept {
        if (quality == SensorQuality::Good || quality == SensorQuality::Warning) {
            quality = SensorQuality::Stale;
        }
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
    TimedValue<double> heading_mag_deg;
    TimedValue<double> depth_m;
    TimedValue<int> gps_fix_quality;
    TimedValue<int> gps_satellites;
};

struct WindState {
    TimedValue<double> awa_deg;
    TimedValue<double> aws_kt;
    TimedValue<double> twa_deg;
    TimedValue<double> tws_kt;
    TimedValue<double> twd_deg;
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
    TimedValue<int> payloads_received;
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

struct SourceStatus {
    LinkStatus status{LinkStatus::Disabled};
    TimedValue<int> valid_sentences;
    TimedValue<int> checksum_errors;
    TimedValue<int> malformed_sentences;
    TimedValue<int> unsupported_sentences;
    std::string endpoint;
    std::string last_error;
};

struct SourceState {
    SourceStatus nmea0183;
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
    SourceState sources;
};

} // namespace helm::telemetry
