#include "telemetry/TelemetryStore.h"

namespace helm::telemetry {

void TelemetryStore::replace_snapshot(const TelemetrySnapshot& snapshot) {
    std::scoped_lock lock(mutex_);
    snapshot_ = snapshot;
}

TelemetrySnapshot TelemetryStore::snapshot() const {
    std::scoped_lock lock(mutex_);
    return snapshot_;
}

} // namespace helm::telemetry
