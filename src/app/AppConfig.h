#pragma once

#include <optional>
#include <string>

namespace helm::app {

struct AppConfig {
    // Empty means use built-in dummy telemetry. Example: tcp-nmea0183://127.0.0.1:10110
    std::optional<std::string> nmea_tcp_source;
};

} // namespace helm::app
