#pragma once

#include "app/AppConfig.h"
#include "nmea/Nmea0183.h"
#include "nmea/NmeaTcpClient.h"
#include "shell/HelmShell.h"
#include "telemetry/DummyTelemetrySource.h"
#include "telemetry/TelemetryStore.h"

#include <gtkmm.h>

namespace helm::app {

class MainWindow final : public Gtk::ApplicationWindow {
public:
    explicit MainWindow(AppConfig config = {});
    ~MainWindow() override;

private:
    bool on_tick();
    void start_live_nmea(const std::string& source_uri);
    void on_nmea_line(std::string line);

    AppConfig config_;
    shell::HelmShell shell_;
    telemetry::TelemetryStore store_;
    telemetry::DummyTelemetrySource dummy_source_;
    nmea::ParserState nmea_parser_state_;
    nmea::NmeaTcpClient nmea_tcp_client_;
    bool use_dummy_{true};
};

} // namespace helm::app
