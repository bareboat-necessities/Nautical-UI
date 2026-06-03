#include "telemetry/TelemetryStore.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>

using namespace helm::telemetry;

int main() {
    TelemetryStore store;
    helm::nmea::TelemetryUpdate update;
    const auto t0 = SteadyClock::now();
    update.timestamp = t0;
    update.has_position = true;
    update.latitude_deg = 40.7;
    update.longitude_deg = -74.0;
    update.has_sog = true;
    update.sog_kt = 6.4;
    update.has_depth = true;
    update.depth_m = 4.5;

    store.apply_nmea_update(update);
    store.record_nmea_parse_status(helm::nmea::ParseStatus::Ok, t0);

    auto s = store.snapshot();
    assert(s.nav.latitude_deg.has_value());
    assert(s.nav.longitude_deg.has_value());
    assert(s.nav.sog_kt.has_value());
    assert(s.nav.depth_m.has_value());
    assert(std::fabs(s.nav.longitude_deg.value - (-74.0)) < 1e-12);
    assert(s.sources.nmea0183.status == LinkStatus::Live);
    assert(s.sources.nmea0183.valid_sentences.value == 1);

    store.refresh_staleness(t0 + std::chrono::seconds(10));
    s = store.snapshot();
    assert(s.nav.sog_kt.quality == SensorQuality::Stale);
    assert(s.nav.depth_m.quality == SensorQuality::Stale);
    assert(s.sources.nmea0183.status == LinkStatus::Stale);

    store.record_nmea_parse_status(helm::nmea::ParseStatus::BadChecksum, t0 + std::chrono::seconds(11));
    s = store.snapshot();
    assert(s.sources.nmea0183.checksum_errors.value == 1);

    std::cout << "telemetry-store-test passed\n";
    return 0;
}
