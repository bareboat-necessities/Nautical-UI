#include "shell/TopStatusBar.h"

#include "telemetry/Format.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace helm::shell {
namespace {

void chip(Gtk::Label& label, const char* css = "status-chip-ok") {
    label.add_css_class("status-chip");
    label.add_css_class(css);
    label.set_margin_top(6);
    label.set_margin_bottom(6);
}

std::string local_time_hhmm() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%H:%M");
    return os.str();
}

} // namespace

TopStatusBar::TopStatusBar()
    : Gtk::Box(Gtk::Orientation::HORIZONTAL, 8), gps_("GPS OK"), nmea_("NMEA DEMO"), ais_("AIS CLEAR"),
      anchor_("⚓ HOLDING"), bilge_("BILGE OK"), battery_("12.7 V"), clock_("--:--") {
    add_css_class("status-bar");
    set_margin_start(8);
    set_margin_end(8);

    for (Gtk::Label* label : {&gps_, &nmea_, &ais_, &anchor_, &bilge_, &battery_, &clock_}) {
        chip(*label);
        append(*label);
    }
    clock_.set_hexpand(true);
    clock_.set_halign(Gtk::Align::END);
}

void TopStatusBar::update(const telemetry::TelemetrySnapshot& snapshot) {
    gps_.set_text(snapshot.nav.latitude_deg.has_value() ? "GPS OK" : "GPS WAIT");
    nmea_.set_text("NMEA DEMO");
    const int danger = snapshot.ais.danger_targets.value;
    ais_.set_text(danger > 0 ? "AIS DANGER" : "AIS CLEAR");
    anchor_.set_text("⚓ " + telemetry::format::anchor_status(snapshot.anchor.status));
    bilge_.set_text("BILGE " + telemetry::format::bilge_status(snapshot.bilge.status));
    battery_.set_text(telemetry::format::volts(snapshot.electrical.house_voltage_v.value));
    clock_.set_text(local_time_hhmm());
}

} // namespace helm::shell
