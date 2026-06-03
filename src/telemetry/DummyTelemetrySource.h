#pragma once

#include "telemetry/TelemetryTypes.h"

#include <chrono>

namespace helm::telemetry {

class DummyTelemetrySource {
public:
    TelemetrySnapshot next_snapshot();

private:
    SteadyTime start_{SteadyClock::now()};
};

} // namespace helm::telemetry
