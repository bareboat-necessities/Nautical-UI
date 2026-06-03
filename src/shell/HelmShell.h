#pragma once

#include "app/AppModule.h"
#include "shell/BottomNavBar.h"
#include "shell/CenterViewport.h"
#include "shell/SensorTile.h"
#include "shell/TopStatusBar.h"

#include <gtkmm.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace helm::shell {

class HelmShell final : public Gtk::Box {
public:
    HelmShell();

    void update(const telemetry::TelemetrySnapshot& snapshot);
    void show_module(const std::string& id);

private:
    template <typename ModuleT>
    void add_module();

    TopStatusBar top_bar_;
    Gtk::Box content_row_;
    Gtk::Box left_tiles_;
    Gtk::Box right_tiles_;
    CenterViewport viewport_;
    BottomNavBar bottom_bar_;

    SensorTile speed_tile_;
    SensorTile depth_tile_;
    SensorTile wind_tile_;
    SensorTile anchor_tile_;
    SensorTile ais_tile_;
    SensorTile bilge_tile_;
    SensorTile battery_tile_;
    SensorTile windlass_tile_;

    std::vector<std::unique_ptr<app::AppModule>> modules_;
    std::unordered_map<std::string, app::AppModule*> module_by_id_;
    std::string active_module_{"home"};
};

} // namespace helm::shell
