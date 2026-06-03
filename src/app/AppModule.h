#pragma once

#include "telemetry/TelemetryTypes.h"

#include <gtkmm.h>
#include <string>

namespace helm::app {

class AppModule {
public:
    virtual ~AppModule() = default;

    [[nodiscard]] virtual std::string id() const = 0;
    [[nodiscard]] virtual std::string title() const = 0;
    [[nodiscard]] virtual Gtk::Widget& widget() = 0;

    virtual void on_activate() {}
    virtual void on_deactivate() {}
    virtual void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) = 0;
};

} // namespace helm::app
