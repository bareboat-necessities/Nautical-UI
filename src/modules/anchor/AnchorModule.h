#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>

namespace helm::modules::anchor {

class AnchorCanvas final : public Gtk::DrawingArea {
public:
    AnchorCanvas();
    void set_snapshot(const telemetry::TelemetrySnapshot& snapshot);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
    telemetry::TelemetrySnapshot snapshot_{};
};

class AnchorModule final : public app::AppModule {
public:
    AnchorModule();

    std::string id() const override { return "anchor"; }
    std::string title() const override { return "Anchor"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    AnchorCanvas canvas_;
    Gtk::Box controls_;
};

} // namespace helm::modules::anchor
