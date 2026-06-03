#pragma once

#include "app/AppModule.h"
#include "telemetry/TelemetryTypes.h"

#include <gtkmm.h>

namespace helm::modules::home {

class HomeCanvas final : public Gtk::DrawingArea {
public:
    HomeCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class HomeModule final : public app::AppModule {
public:
    HomeModule();

    std::string id() const override { return "home"; }
    std::string title() const override { return "Home"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    HomeCanvas canvas_;
};

} // namespace helm::modules::home
