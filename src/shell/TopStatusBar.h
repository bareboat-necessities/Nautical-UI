#pragma once

#include "telemetry/TelemetryTypes.h"

#include <gtkmm.h>

namespace helm::shell {

class TopStatusBar final : public Gtk::Box {
public:
    TopStatusBar();
    void update(const telemetry::TelemetrySnapshot& snapshot);

private:
    Gtk::Label gps_;
    Gtk::Label nmea_;
    Gtk::Label ais_;
    Gtk::Label anchor_;
    Gtk::Label bilge_;
    Gtk::Label battery_;
    Gtk::Label clock_;
};

} // namespace helm::shell
