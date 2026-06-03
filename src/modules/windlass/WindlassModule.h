#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::windlass {

class WindlassCanvas final : public Gtk::DrawingArea {
public:
    WindlassCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class WindlassModule final : public app::AppModule {
public:
    WindlassModule();

    std::string id() const override { return "windlass"; }
    std::string title() const override { return "Windlass"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    WindlassCanvas canvas_;
    Gtk::Box controls_;
};

} // namespace helm::modules::windlass
