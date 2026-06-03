#include "telemetry/TelemetryStore.h"

#include <utility>

namespace helm::telemetry {
namespace {

template <typename T>
void set_value(TimedValue<T>& tv, T value, SteadyTime when, std::string_view source,
               SensorQuality quality = SensorQuality::Good) {
    tv.value = std::move(value);
    tv.updated = when;
    tv.quality = quality;
    tv.source.assign(source.begin(), source.end());
}

template <typename T>
void stale_if_old(TimedValue<T>& tv, SteadyTime now, std::chrono::milliseconds age) {
    if (tv.has_value() && tv.is_stale(now, age)) {
        tv.mark_stale();
    }
}

bool is_error_status(nmea::ParseStatus status) {
    return status == nmea::ParseStatus::BadChecksum || status == nmea::ParseStatus::Malformed ||
           status == nmea::ParseStatus::BadStart || status == nmea::ParseStatus::TooLong;
}

} // namespace

void TelemetryStore::replace_snapshot(const TelemetrySnapshot& snapshot) {
    std::scoped_lock lock(mutex_);
    snapshot_ = snapshot;
}

void TelemetryStore::apply_nmea_update(const nmea::TelemetryUpdate& u, std::string_view source) {
    std::scoped_lock lock(mutex_);
    snapshot_.timestamp = u.timestamp;
    snapshot_.sources.nmea0183.status = LinkStatus::Live;

    if (u.has_position) {
        set_value(snapshot_.nav.latitude_deg, u.latitude_deg, u.timestamp, source);
        set_value(snapshot_.nav.longitude_deg, u.longitude_deg, u.timestamp, source);
    }
    if (u.has_sog) {
        set_value(snapshot_.nav.sog_kt, u.sog_kt, u.timestamp, source);
    }
    if (u.has_cog) {
        set_value(snapshot_.nav.cog_deg, u.cog_deg, u.timestamp, source);
    }
    if (u.has_heading_true) {
        set_value(snapshot_.nav.heading_true_deg, u.heading_true_deg, u.timestamp, source);
    }
    if (u.has_heading_mag) {
        set_value(snapshot_.nav.heading_mag_deg, u.heading_mag_deg, u.timestamp, source);
    }
    if (u.has_depth) {
        set_value(snapshot_.nav.depth_m, u.depth_m, u.timestamp, source);
    }
    if (u.has_wind_apparent) {
        set_value(snapshot_.wind.awa_deg, u.awa_deg, u.timestamp, source);
        set_value(snapshot_.wind.aws_kt, u.aws_kt, u.timestamp, source);
    }
    if (u.has_wind_true) {
        set_value(snapshot_.wind.twa_deg, u.twa_deg, u.timestamp, source);
        set_value(snapshot_.wind.tws_kt, u.tws_kt, u.timestamp, source);
    }
    if (u.has_wind_direction) {
        set_value(snapshot_.wind.twd_deg, u.twd_deg, u.timestamp, source);
        set_value(snapshot_.wind.tws_kt, u.tws_kt, u.timestamp, source);
    }
    if (u.has_gps_fix_quality) {
        set_value(snapshot_.nav.gps_fix_quality, u.gps_fix_quality, u.timestamp, source,
                  u.gps_fix_quality > 0 ? SensorQuality::Good : SensorQuality::Warning);
        set_value(snapshot_.nav.gps_satellites, u.satellites, u.timestamp, source,
                  u.satellites > 3 ? SensorQuality::Good : SensorQuality::Warning);
    }
    if (u.has_ais_payload) {
        const int count = snapshot_.ais.payloads_received.has_value() ? snapshot_.ais.payloads_received.value + 1 : 1;
        set_value(snapshot_.ais.payloads_received, count, u.timestamp, source);
        // Full AIS target decoding is Phase 5. Phase 3 keeps the assembled payload visible to the data layer.
        if (!snapshot_.ais.targets.has_value()) {
            set_value(snapshot_.ais.targets, 0, u.timestamp, source);
            set_value(snapshot_.ais.danger_targets, 0, u.timestamp, source);
        }
    }
}

void TelemetryStore::record_nmea_parse_status(nmea::ParseStatus status, SteadyTime now) {
    std::scoped_lock lock(mutex_);
    snapshot_.timestamp = now;

    if (status == nmea::ParseStatus::Ok) {
        ++nmea_valid_count_;
        set_value(snapshot_.sources.nmea0183.valid_sentences, nmea_valid_count_, now, "nmea0183");
        snapshot_.sources.nmea0183.status = LinkStatus::Live;
        snapshot_.sources.nmea0183.last_error.clear();
        return;
    }

    if (status == nmea::ParseStatus::Unsupported) {
        ++nmea_unsupported_count_;
        set_value(snapshot_.sources.nmea0183.unsupported_sentences, nmea_unsupported_count_, now, "nmea0183",
                  SensorQuality::Warning);
        return;
    }

    if (status == nmea::ParseStatus::BadChecksum) {
        ++nmea_checksum_error_count_;
        set_value(snapshot_.sources.nmea0183.checksum_errors, nmea_checksum_error_count_, now, "nmea0183",
                  SensorQuality::Warning);
        snapshot_.sources.nmea0183.last_error = "bad checksum";
    } else if (is_error_status(status)) {
        ++nmea_malformed_count_;
        set_value(snapshot_.sources.nmea0183.malformed_sentences, nmea_malformed_count_, now, "nmea0183",
                  SensorQuality::Warning);
        snapshot_.sources.nmea0183.last_error = nmea::to_string(status);
    }
}

void TelemetryStore::set_nmea_endpoint(std::string endpoint) {
    std::scoped_lock lock(mutex_);
    snapshot_.sources.nmea0183.endpoint = std::move(endpoint);
    if (!snapshot_.sources.nmea0183.endpoint.empty() && snapshot_.sources.nmea0183.status == LinkStatus::Disabled) {
        snapshot_.sources.nmea0183.status = LinkStatus::Connecting;
    }
}

void TelemetryStore::set_nmea_link_status(LinkStatus status, std::string error) {
    std::scoped_lock lock(mutex_);
    snapshot_.timestamp = SteadyClock::now();
    snapshot_.sources.nmea0183.status = status;
    snapshot_.sources.nmea0183.last_error = std::move(error);
}

void TelemetryStore::refresh_staleness(SteadyTime now) {
    std::scoped_lock lock(mutex_);
    refresh_staleness_locked(now);
}

TelemetrySnapshot TelemetryStore::snapshot() const {
    std::scoped_lock lock(mutex_);
    return snapshot_;
}

void TelemetryStore::refresh_staleness_locked(SteadyTime now) {
    snapshot_.timestamp = now;

    stale_if_old(snapshot_.nav.latitude_deg, now, GpsStaleAfter);
    stale_if_old(snapshot_.nav.longitude_deg, now, GpsStaleAfter);
    stale_if_old(snapshot_.nav.sog_kt, now, GpsStaleAfter);
    stale_if_old(snapshot_.nav.cog_deg, now, GpsStaleAfter);
    stale_if_old(snapshot_.nav.gps_fix_quality, now, GpsStaleAfter);
    stale_if_old(snapshot_.nav.gps_satellites, now, GpsStaleAfter);

    stale_if_old(snapshot_.nav.heading_true_deg, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.nav.heading_mag_deg, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.nav.depth_m, now, SystemsStaleAfter);

    stale_if_old(snapshot_.wind.awa_deg, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.wind.aws_kt, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.wind.twa_deg, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.wind.tws_kt, now, DefaultSensorStaleAfter);
    stale_if_old(snapshot_.wind.twd_deg, now, DefaultSensorStaleAfter);

    stale_if_old(snapshot_.ais.targets, now, AisStaleAfter);
    stale_if_old(snapshot_.ais.danger_targets, now, AisStaleAfter);
    stale_if_old(snapshot_.ais.nearest_range_nm, now, AisStaleAfter);
    stale_if_old(snapshot_.ais.threat_cpa_nm, now, AisStaleAfter);
    stale_if_old(snapshot_.ais.threat_tcpa_min, now, AisStaleAfter);
    stale_if_old(snapshot_.ais.payloads_received, now, AisStaleAfter);

    stale_if_old(snapshot_.bilge.high_water, now, SystemsStaleAfter);
    stale_if_old(snapshot_.bilge.pump_on, now, SystemsStaleAfter);
    stale_if_old(snapshot_.bilge.pump_current_a, now, SystemsStaleAfter);
    stale_if_old(snapshot_.bilge.runtime_today_min, now, SystemsStaleAfter);

    stale_if_old(snapshot_.windlass.chain_deployed_m, now, SystemsStaleAfter);
    stale_if_old(snapshot_.windlass.motor_current_a, now, SystemsStaleAfter);
    stale_if_old(snapshot_.windlass.breaker_on, now, SystemsStaleAfter);

    stale_if_old(snapshot_.electrical.house_voltage_v, now, SystemsStaleAfter);
    stale_if_old(snapshot_.electrical.house_current_a, now, SystemsStaleAfter);

    if (snapshot_.sources.nmea0183.status == LinkStatus::Live &&
        snapshot_.sources.nmea0183.valid_sentences.has_value() &&
        snapshot_.sources.nmea0183.valid_sentences.is_stale(now, DefaultSensorStaleAfter)) {
        snapshot_.sources.nmea0183.status = LinkStatus::Stale;
    }
}

} // namespace helm::telemetry
