#pragma once

#include "telemetry/TelemetryTypes.h"

#include <string>

namespace helm::telemetry::format {

std::string fixed(double value, int decimals);
std::string degrees(double value, int decimals = 0);
std::string knots(double value, int decimals = 1);
std::string meters(double value, int decimals = 1);
std::string nautical_miles(double value, int decimals = 2);
std::string volts(double value, int decimals = 1);
std::string amps(double value, int decimals = 1);
std::string quality_label(SensorQuality q);
std::string anchor_status(AnchorStatus s);
std::string bilge_status(BilgeStatus s);
std::string windlass_status(WindlassStatus s);

} // namespace helm::telemetry::format
