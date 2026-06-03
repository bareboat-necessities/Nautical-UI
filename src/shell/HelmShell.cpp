#include "shell/HelmShell.h"

#include "modules/ais/AisModule.h"
#include "modules/anchor/AnchorModule.h"
#include "modules/bilge/BilgeModule.h"
#include "modules/home/HomeModule.h"
#include "modules/systems/SystemsModule.h"
#include "modules/windlass/WindlassModule.h"
#include "telemetry/Format.h"

namespace helm::shell {

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
    add_module<modules::anchor::AnchorModule>();
    add_module<modules::ais::AisModule>();
    add_module<modules::systems::SystemsModule>();
    add_module<modules::bilge::BilgeModule>();
    add_module<modules::windlass::WindlassModule>();

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

    speed_tile_.set_value(telemetry::format::knots(snapshot.nav.sog_kt.value, 1));
    speed_tile_.set_subtitle("COG " + telemetry::format::degrees(snapshot.nav.cog_deg.value, 0));
    speed_tile_.set_state(graphics::IconState::Ok);

    depth_tile_.set_value(telemetry::format::meters(snapshot.nav.depth_m.value, 1));
    depth_tile_.set_subtitle("below transducer");
    depth_tile_.set_state(snapshot.nav.depth_m.value < 3.0 ? graphics::IconState::Danger : graphics::IconState::Ok);

    wind_tile_.set_value(telemetry::format::knots(snapshot.wind.tws_kt.value, 1));
    wind_tile_.set_subtitle("TWA " + telemetry::format::degrees(snapshot.wind.twa_deg.value, 0));
    wind_tile_.set_state(graphics::IconState::Ok);

    anchor_tile_.set_value(telemetry::format::meters(snapshot.anchor.distance_m.value, 0));
    anchor_tile_.set_subtitle("radius " + telemetry::format::meters(snapshot.anchor.radius_m.value, 0));
    anchor_tile_.set_state(graphics::IconState::Ok);

    ais_tile_.set_value(std::to_string(snapshot.ais.targets.value));
    ais_tile_.set_subtitle(snapshot.ais.danger_targets.value > 0 ? "DANGER" : "clear");
    ais_tile_.set_state(snapshot.ais.danger_targets.value > 0 ? graphics::IconState::Danger : graphics::IconState::Ok);

    bilge_tile_.set_value(telemetry::format::bilge_status(snapshot.bilge.status));
    bilge_tile_.set_subtitle(snapshot.bilge.pump_on.value ? "pump running" : "auto ok");
    bilge_tile_.set_state(snapshot.bilge.status == telemetry::BilgeStatus::Dry ? graphics::IconState::Ok : graphics::IconState::Warning);

    battery_tile_.set_value(telemetry::format::volts(snapshot.electrical.house_voltage_v.value, 1));
    battery_tile_.set_subtitle(telemetry::format::amps(snapshot.electrical.house_current_a.value, 1));
    battery_tile_.set_state(snapshot.electrical.house_voltage_v.value < 12.0 ? graphics::IconState::Warning : graphics::IconState::Ok);

    windlass_tile_.set_value(telemetry::format::windlass_status(snapshot.windlass.status));
    windlass_tile_.set_subtitle(telemetry::format::meters(snapshot.windlass.chain_deployed_m.value, 0));
    windlass_tile_.set_state(graphics::IconState::Warning);

    for (auto& module : modules_) {
        module->on_telemetry(snapshot);
    }
}

} // namespace helm::shell
