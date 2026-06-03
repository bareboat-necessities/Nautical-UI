#pragma once

#include "nmea/Nmea0183.h"
#include "telemetry/TelemetryTypes.h"

#include <mutex>
#include <string>
#include <string_view>

namespace helm::telemetry {

class TelemetryStore {
public:
    void replace_snapshot(const TelemetrySnapshot& snapshot);
    void apply_nmea_update(const nmea::TelemetryUpdate& update, std::string_view source = "nmea0183");
    void record_nmea_parse_status(nmea::ParseStatus status, SteadyTime now);
    void set_nmea_endpoint(std::string endpoint);
    void set_nmea_link_status(LinkStatus status, std::string error = {});
    void refresh_staleness(SteadyTime now = SteadyClock::now());

    [[nodiscard]] TelemetrySnapshot snapshot() const;

private:
    void refresh_staleness_locked(SteadyTime now);

    mutable std::mutex mutex_;
    TelemetrySnapshot snapshot_{};
    int nmea_valid_count_{0};
    int nmea_checksum_error_count_{0};
    int nmea_malformed_count_{0};
    int nmea_unsupported_count_{0};
};

} // namespace helm::telemetry
