#include "telemetry/Format.h"

#include <iomanip>
#include <sstream>

namespace helm::telemetry::format {

std::string fixed(double value, int decimals) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(decimals) << value;
    return os.str();
}

std::string degrees(double value, int decimals) {
    return fixed(value, decimals) + "°";
}

std::string knots(double value, int decimals) {
    return fixed(value, decimals) + " kt";
}

std::string meters(double value, int decimals) {
    return fixed(value, decimals) + " m";
}

std::string nautical_miles(double value, int decimals) {
    return fixed(value, decimals) + " nm";
}

std::string volts(double value, int decimals) {
    return fixed(value, decimals) + " V";
}

std::string amps(double value, int decimals) {
    return fixed(value, decimals) + " A";
}

std::string quality_label(SensorQuality q) {
    switch (q) {
        case SensorQuality::Missing: return "MISSING";
        case SensorQuality::Stale: return "STALE";
        case SensorQuality::Good: return "OK";
        case SensorQuality::Warning: return "WARN";
        case SensorQuality::Alarm: return "ALARM";
    }
    return "?";
}

std::string anchor_status(AnchorStatus s) {
    switch (s) {
        case AnchorStatus::Disarmed: return "DISARMED";
        case AnchorStatus::HoldingOk: return "HOLDING OK";
        case AnchorStatus::NearLimit: return "NEAR LIMIT";
        case AnchorStatus::DragWarning: return "DRAG WARNING";
        case AnchorStatus::DragAlarm: return "DRAG ALARM";
        case AnchorStatus::GpsLost: return "GPS LOST";
    }
    return "?";
}

std::string bilge_status(BilgeStatus s) {
    switch (s) {
        case BilgeStatus::Dry: return "DRY";
        case BilgeStatus::PumpRunning: return "PUMP ON";
        case BilgeStatus::HighWater: return "HIGH WATER";
        case BilgeStatus::Fault: return "FAULT";
    }
    return "?";
}

std::string windlass_status(WindlassStatus s) {
    switch (s) {
        case WindlassStatus::Locked: return "LOCKED";
        case WindlassStatus::Armed: return "ARMED";
        case WindlassStatus::RunningUp: return "UP";
        case WindlassStatus::RunningDown: return "DOWN";
        case WindlassStatus::Fault: return "FAULT";
    }
    return "?";
}

} // namespace helm::telemetry::format
