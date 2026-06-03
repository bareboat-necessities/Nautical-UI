#include "modules/alarms/AlarmModule.h"

#include "telemetry/Format.h"

namespace helm::modules::alarms {
namespace {

void prepare_label(Gtk::Label& label, float xalign = 0.0f) {
    label.set_xalign(xalign);
    label.set_wrap(true);
}

} // namespace

AlarmModule::AlarmModule()
    : root_(Gtk::Orientation::VERTICAL, 10), title_("ALARMS"),
      subtitle_("Global alarm center. Safety alarms must override media/messages and ordinary screens."),
      list_(Gtk::Orientation::VERTICAL, 8), controls_(Gtk::Orientation::HORIZONTAL, 8) {
    root_.set_margin_top(16);
    root_.set_margin_bottom(16);
    root_.set_margin_start(16);
    root_.set_margin_end(16);

    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    subtitle_.add_css_class("module-subtitle");
    subtitle_.set_xalign(0.0f);

    auto* ack = Gtk::make_managed<Gtk::Button>("ACK SELECTED");
    auto* silence = Gtk::make_managed<Gtk::Button>("SILENCE 2 MIN");
    auto* log = Gtk::make_managed<Gtk::Button>("ALARM LOG");
    for (auto* b : {ack, silence, log}) {
        b->add_css_class("control-button");
        controls_.append(*b);
    }

    root_.append(title_);
    root_.append(subtitle_);
    root_.append(list_);
    root_.append(controls_);
}

void AlarmModule::clear_rows() {
    for (auto* row : row_widgets_) {
        list_.remove(*row);
    }
    row_widgets_.clear();
}

void AlarmModule::append_alarm(const std::string& severity, const std::string& title, const std::string& detail, const char* css_class) {
    auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 12);
    row->add_css_class("alarm-card");
    row->add_css_class(css_class);
    row->set_margin_top(2);
    row->set_margin_bottom(2);

    auto* sev = Gtk::make_managed<Gtk::Label>(severity);
    auto* texts = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 4);
    auto* heading = Gtk::make_managed<Gtk::Label>(title);
    auto* body = Gtk::make_managed<Gtk::Label>(detail);

    sev->add_css_class("alarm-severity");
    sev->set_size_request(130, -1);
    prepare_label(*sev, 0.5f);
    heading->add_css_class("alarm-title");
    body->add_css_class("alarm-detail");
    prepare_label(*heading);
    prepare_label(*body);

    texts->append(*heading);
    texts->append(*body);
    texts->set_hexpand(true);

    row->append(*sev);
    row->append(*texts);
    list_.append(*row);
    row_widgets_.push_back(row);
}

void AlarmModule::rebuild_rows(const telemetry::TelemetrySnapshot& s) {
    clear_rows();

    if (s.nav.depth_m.has_value() && s.nav.depth_m.value < 2.5) {
        append_alarm("CRITICAL", "SHALLOW WATER",
                     "Depth is " + telemetry::format::meters(s.nav.depth_m.value, 1) +
                         ". Check under-keel clearance and course immediately.",
                     "alarm-critical");
    } else if (s.nav.depth_m.has_value() && s.nav.depth_m.value < 4.0) {
        append_alarm("WARNING", "DEPTH CAUTION",
                     "Depth is " + telemetry::format::meters(s.nav.depth_m.value, 1) +
                         ". Shallow-water warning threshold is approaching.",
                     "alarm-warning");
    }

    if (s.ais.danger_targets.has_value() && s.ais.danger_targets.value > 0) {
        append_alarm("CRITICAL", "AIS COLLISION RISK",
                     "Threat CPA " + telemetry::format::nautical_miles(s.ais.threat_cpa_nm.value, 2) +
                         " in " + telemetry::format::fixed(s.ais.threat_tcpa_min.value, 0) +
                         " min. Open AIS or NAV screen.",
                     "alarm-critical");
    }

    if (s.anchor.status == telemetry::AnchorStatus::DragAlarm || s.anchor.status == telemetry::AnchorStatus::DragWarning) {
        append_alarm(s.anchor.status == telemetry::AnchorStatus::DragAlarm ? "CRITICAL" : "WARNING", "ANCHOR DRAG",
                     "Anchor distance " + telemetry::format::meters(s.anchor.distance_m.value, 0) + " / radius " +
                         telemetry::format::meters(s.anchor.radius_m.value, 0) + ".",
                     s.anchor.status == telemetry::AnchorStatus::DragAlarm ? "alarm-critical" : "alarm-warning");
    }

    if (s.bilge.status == telemetry::BilgeStatus::HighWater || s.bilge.status == telemetry::BilgeStatus::Fault) {
        append_alarm("CRITICAL", "BILGE HIGH WATER",
                     "Bilge status is " + telemetry::format::bilge_status(s.bilge.status) +
                         ". Pump current " + telemetry::format::amps(s.bilge.pump_current_a.value, 1) + ".",
                     "alarm-critical");
    } else if (s.bilge.status == telemetry::BilgeStatus::PumpRunning) {
        append_alarm("INFO", "BILGE PUMP RUNNING",
                     "Pump current " + telemetry::format::amps(s.bilge.pump_current_a.value, 1) +
                         ". Runtime today " + telemetry::format::fixed(s.bilge.runtime_today_min.value, 1) + " min.",
                     "alarm-info");
    }

    if (s.windlass.status == telemetry::WindlassStatus::Fault) {
        append_alarm("CRITICAL", "WINDLASS FAULT",
                     "Windlass fault detected. Check breaker, current, and physical controls.", "alarm-critical");
    }

    if (s.electrical.house_voltage_v.has_value() && s.electrical.house_voltage_v.value < 12.0) {
        append_alarm("WARNING", "LOW HOUSE BATTERY",
                     "House voltage " + telemetry::format::volts(s.electrical.house_voltage_v.value, 1) + ".",
                     "alarm-warning");
    }

    if (s.sources.nmea0183.status == telemetry::LinkStatus::Error || s.sources.nmea0183.status == telemetry::LinkStatus::Stale) {
        append_alarm("WARNING", "NMEA INPUT", s.sources.nmea0183.last_error.empty() ? "NMEA 0183 stream is not live." : s.sources.nmea0183.last_error,
                     "alarm-warning");
    }

    if (row_widgets_.empty()) {
        append_alarm("OK", "NO ACTIVE ALARMS",
                     "Depth, AIS, anchor, bilge, windlass, battery, and NMEA link are currently clear.",
                     "alarm-info");
    }
}

void AlarmModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    rebuild_rows(snapshot);
}

} // namespace helm::modules::alarms
