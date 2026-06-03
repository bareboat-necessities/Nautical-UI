#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::sail {

class SailCanvas final : public Gtk::DrawingArea {
public:
    SailCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class SailModule final : public app::AppModule {
public:
    SailModule();

    std::string id() const override { return "sail"; }
    std::string title() const override { return "Sail"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    SailCanvas canvas_;
};

} // namespace helm::modules::sail
