#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::bilge {

class BilgeCanvas final : public Gtk::DrawingArea {
public:
    BilgeCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class BilgeModule final : public app::AppModule {
public:
    BilgeModule();

    std::string id() const override { return "bilge"; }
    std::string title() const override { return "Bilge"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    BilgeCanvas canvas_;
    Gtk::Box controls_;
};

} // namespace helm::modules::bilge
