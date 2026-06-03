#include "shell/HelmShell.h"

#include "modules/ais/AisModule.h"
#include "modules/alarms/AlarmModule.h"
#include "modules/anchor/AnchorModule.h"
#include "modules/bilge/BilgeModule.h"
#include "modules/nav/NavModule.h"
#include "modules/sail/SailModule.h"
#include "modules/home/HomeModule.h"
#include "modules/systems/SystemsModule.h"
#include "modules/windlass/WindlassModule.h"
#include "telemetry/Format.h"

namespace helm::shell {
namespace {

graphics::IconState icon_state_for_quality(telemetry::SensorQuality q, graphics::IconState good = graphics::IconState::Ok) {
    switch (q) {
        case telemetry::SensorQuality::Good: return good;
        case telemetry::SensorQuality::Warning: return graphics::IconState::Warning;
        case telemetry::SensorQuality::Alarm: return graphics::IconState::Danger;
        case telemetry::SensorQuality::Stale: return graphics::IconState::Warning;
        case telemetry::SensorQuality::Missing: return graphics::IconState::Disabled;
    }
    return graphics::IconState::Disabled;
}

template <typename T, typename Formatter>
std::string format_or_dash(const telemetry::TimedValue<T>& tv, Formatter formatter) {
    if (!tv.has_value()) {
        return "--";
    }
    return formatter(tv.value);
}

} // namespace

HelmShell::HelmShell()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 8), content_row_(Gtk::Orientation::HORIZONTAL, 8),
      left_tiles_(Gtk::Orientation::VERTICAL, 8), right_tiles_(Gtk::Orientation::VERTICAL, 8),
      speed_tile_(graphics::IconKind::Speed, "SOG"), depth_tile_(graphics::IconKind::Depth, "DEPTH"),
      wind_tile_(graphics::IconKind::Wind, "WIND"), anchor_tile_(graphics::IconKind::Anchor, "ANCHOR"),
      ais_tile_(graphics::IconKind::Ais, "AIS"), bilge_tile_(graphics::IconKind::Bilge, "BILGE"),
      battery_tile_(graphics::IconKind::Battery, "HOUSE"), windlass_tile_(graphics::IconKind::Windlass, "WINDLASS") {
    add_css_class("helm-shell");
    set_hexpand(true);
    set_vexpand(true);

    left_tiles_.set_margin_start(8);
    right_tiles_.set_margin_end(8);
    left_tiles_.append(speed_tile_);
    left_tiles_.append(depth_tile_);
    left_tiles_.append(wind_tile_);
    left_tiles_.append(anchor_tile_);

    right_tiles_.append(ais_tile_);
    right_tiles_.append(bilge_tile_);
    right_tiles_.append(battery_tile_);
    right_tiles_.append(windlass_tile_);

    add_module<modules::home::HomeModule>();
    add_module<modules::nav::NavModule>();
    add_module<modules::sail::SailModule>();
    add_module<modules::anchor::AnchorModule>();
    add_module<modules::ais::AisModule>();
    add_module<modules::systems::SystemsModule>();
    add_module<modules::bilge::BilgeModule>();
    add_module<modules::windlass::WindlassModule>();
    add_module<modules::alarms::AlarmModule>();

    content_row_.append(left_tiles_);
    content_row_.append(viewport_);
    content_row_.append(right_tiles_);
    content_row_.set_vexpand(true);

    bottom_bar_.set_on_selected([this](std::string id) { show_module(id); });

    append(top_bar_);
    append(content_row_);
    append(bottom_bar_);

    show_module("home");
}

template <typename ModuleT>
void HelmShell::add_module() {
    auto module = std::make_unique<ModuleT>();
    auto* ptr = module.get();
    module_by_id_[ptr->id()] = ptr;
    viewport_.add_module(*ptr);
    modules_.push_back(std::move(module));
}

void HelmShell::show_module(const std::string& id) {
    if (auto it = module_by_id_.find(active_module_); it != module_by_id_.end()) {
        it->second->on_deactivate();
    }
    active_module_ = id;
    viewport_.show_module(id);
    bottom_bar_.set_active(id);
    if (auto it = module_by_id_.find(active_module_); it != module_by_id_.end()) {
        it->second->on_activate();
    }
}

void HelmShell::update(const telemetry::TelemetrySnapshot& snapshot) {
    top_bar_.update(snapshot);

    speed_tile_.set_value(format_or_dash(snapshot.nav.sog_kt, [](double v) { return telemetry::format::knots(v, 1); }));
    speed_tile_.set_subtitle("COG " + format_or_dash(snapshot.nav.cog_deg, [](double v) { return telemetry::format::degrees(v, 0); }));
    speed_tile_.set_state(icon_state_for_quality(snapshot.nav.sog_kt.quality));

    depth_tile_.set_value(format_or_dash(snapshot.nav.depth_m, [](double v) { return telemetry::format::meters(v, 1); }));
    depth_tile_.set_subtitle(snapshot.nav.depth_m.quality == telemetry::SensorQuality::Stale ? "stale" : "below transducer");
    if (snapshot.nav.depth_m.has_value() && snapshot.nav.depth_m.value < 3.0) {
        depth_tile_.set_state(graphics::IconState::Danger);
    } else {
        depth_tile_.set_state(icon_state_for_quality(snapshot.nav.depth_m.quality));
    }

    wind_tile_.set_value(format_or_dash(snapshot.wind.tws_kt, [](double v) { return telemetry::format::knots(v, 1); }));
    wind_tile_.set_subtitle("TWA " + format_or_dash(snapshot.wind.twa_deg, [](double v) { return telemetry::format::degrees(v, 0); }));
    wind_tile_.set_state(icon_state_for_quality(snapshot.wind.tws_kt.quality));

    anchor_tile_.set_value(format_or_dash(snapshot.anchor.distance_m, [](double v) { return telemetry::format::meters(v, 0); }));
    anchor_tile_.set_subtitle("radius " + format_or_dash(snapshot.anchor.radius_m, [](double v) { return telemetry::format::meters(v, 0); }));
    anchor_tile_.set_state(snapshot.anchor.status == telemetry::AnchorStatus::DragAlarm ? graphics::IconState::Danger : graphics::IconState::Ok);

    ais_tile_.set_value(format_or_dash(snapshot.ais.targets, [](int v) { return std::to_string(v); }));
    const int danger = snapshot.ais.danger_targets.has_value() ? snapshot.ais.danger_targets.value : 0;
    ais_tile_.set_subtitle(danger > 0 ? "DANGER" : "clear");
    ais_tile_.set_state(danger > 0 ? graphics::IconState::Danger : icon_state_for_quality(snapshot.ais.targets.quality));

    bilge_tile_.set_value(telemetry::format::bilge_status(snapshot.bilge.status));
    bilge_tile_.set_subtitle(snapshot.bilge.pump_on.has_value() && snapshot.bilge.pump_on.value ? "pump running" : "auto ok");
    bilge_tile_.set_state(snapshot.bilge.status == telemetry::BilgeStatus::Dry ? graphics::IconState::Ok : graphics::IconState::Warning);

    battery_tile_.set_value(format_or_dash(snapshot.electrical.house_voltage_v, [](double v) { return telemetry::format::volts(v, 1); }));
    battery_tile_.set_subtitle(format_or_dash(snapshot.electrical.house_current_a, [](double v) { return telemetry::format::amps(v, 1); }));
    if (snapshot.electrical.house_voltage_v.has_value() && snapshot.electrical.house_voltage_v.value < 12.0) {
        battery_tile_.set_state(graphics::IconState::Warning);
    } else {
        battery_tile_.set_state(icon_state_for_quality(snapshot.electrical.house_voltage_v.quality));
    }

    windlass_tile_.set_value(telemetry::format::windlass_status(snapshot.windlass.status));
    windlass_tile_.set_subtitle(format_or_dash(snapshot.windlass.chain_deployed_m, [](double v) { return telemetry::format::meters(v, 0); }));
    windlass_tile_.set_state(snapshot.windlass.status == telemetry::WindlassStatus::Locked ? graphics::IconState::Ok :
                              (snapshot.windlass.status == telemetry::WindlassStatus::Fault ? graphics::IconState::Danger : graphics::IconState::Warning));

    for (auto& module : modules_) {
        module->on_telemetry(snapshot);
    }
}

} // namespace helm::shell
