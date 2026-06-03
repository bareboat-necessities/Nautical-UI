#pragma once

#include "shell/HelmShell.h"
#include "telemetry/DummyTelemetrySource.h"
#include "telemetry/TelemetryStore.h"

#include <gtkmm.h>

namespace helm::app {

class MainWindow final : public Gtk::ApplicationWindow {
public:
    MainWindow();

private:
    bool on_tick();

    shell::HelmShell shell_;
    telemetry::TelemetryStore store_;
    telemetry::DummyTelemetrySource dummy_source_;
};

} // namespace helm::app
