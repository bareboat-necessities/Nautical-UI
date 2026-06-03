#include "modules/systems/SystemsModule.h"

#include "telemetry/Format.h"

namespace helm::modules::systems {
namespace {

void prepare_card(Gtk::Label& label) {
    label.add_css_class("sensor-tile");
    label.set_xalign(0.0f);
    label.set_margin_top(4);
    label.set_margin_bottom(4);
    label.set_margin_start(4);
    label.set_margin_end(4);
}

} // namespace

SystemsModule::SystemsModule()
    : root_(Gtk::Orientation::VERTICAL, 10), title_("SYSTEMS"), electrical_("Electrical"), bilge_("Bilge"),
      windlass_("Windlass"), ais_("AIS") {
    root_.set_margin_top(16);
    root_.set_margin_bottom(16);
    root_.set_margin_start(16);
    root_.set_margin_end(16);
    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    root_.append(title_);

    for (Gtk::Label* label : {&electrical_, &bilge_, &windlass_, &ais_}) {
        prepare_card(*label);
        root_.append(*label);
    }
}

void SystemsModule::on_telemetry(const telemetry::TelemetrySnapshot& s) {
    electrical_.set_text("ELECTRICAL\nHouse " + telemetry::format::volts(s.electrical.house_voltage_v.value, 1) +
                         "   Load " + telemetry::format::amps(s.electrical.house_current_a.value, 1));
    bilge_.set_text("BILGE\n" + telemetry::format::bilge_status(s.bilge.status) +
                    "   Pump " + std::string(s.bilge.pump_on.value ? "ON" : "OFF") +
                    "   Current " + telemetry::format::amps(s.bilge.pump_current_a.value, 1));
    windlass_.set_text("WINDLASS\n" + telemetry::format::windlass_status(s.windlass.status) +
                       "   Chain " + telemetry::format::meters(s.windlass.chain_deployed_m.value, 0));
    ais_.set_text("AIS\nTargets " + std::to_string(s.ais.targets.value) +
                  "   Nearest " + telemetry::format::nautical_miles(s.ais.nearest_range_nm.value, 2));
}

} // namespace helm::modules::systems
