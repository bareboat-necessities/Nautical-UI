#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::systems {

class SystemsModule final : public app::AppModule {
public:
    SystemsModule();

    std::string id() const override { return "systems"; }
    std::string title() const override { return "Systems"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label electrical_;
    Gtk::Label bilge_;
    Gtk::Label windlass_;
    Gtk::Label ais_;
};

} // namespace helm::modules::systems
