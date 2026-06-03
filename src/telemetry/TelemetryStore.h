#pragma once

#include "telemetry/TelemetryTypes.h"

#include <mutex>

namespace helm::telemetry {

class TelemetryStore {
public:
    void replace_snapshot(const TelemetrySnapshot& snapshot);
    [[nodiscard]] TelemetrySnapshot snapshot() const;

private:
    mutable std::mutex mutex_;
    TelemetrySnapshot snapshot_{};
};

} // namespace helm::telemetry
