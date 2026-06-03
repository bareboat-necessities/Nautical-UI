#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::ais {

class AisCanvas final : public Gtk::DrawingArea {
public:
    AisCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class AisModule final : public app::AppModule {
public:
    AisModule();

    std::string id() const override { return "ais"; }
    std::string title() const override { return "AIS"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    AisCanvas canvas_;
};

} // namespace helm::modules::ais
