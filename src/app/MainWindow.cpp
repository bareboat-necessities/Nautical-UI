#include "app/MainWindow.h"

#include "graphics/Theme.h"

#include <glibmm/main.h>

#include <utility>

namespace helm::app {

MainWindow::MainWindow(AppConfig config) : config_(std::move(config)) {
    graphics::install_css();
    set_title("Helm UI GTK4");
    set_default_size(1280, 800);
    set_child(shell_);

    if (config_.nmea_tcp_source.has_value()) {
        start_live_nmea(*config_.nmea_tcp_source);
    }

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_tick), 100);
    on_tick();
}

MainWindow::~MainWindow() {
    nmea_tcp_client_.stop();
}

void MainWindow::start_live_nmea(const std::string& source_uri) {
    nmea::TcpEndpoint endpoint;
    std::string error;
    if (!nmea::parse_tcp_nmea0183_uri(source_uri, endpoint, error)) {
        store_.set_nmea_endpoint(source_uri);
        store_.set_nmea_link_status(telemetry::LinkStatus::Error, error);
        use_dummy_ = true;
        return;
    }

    use_dummy_ = false;
    store_.set_nmea_endpoint(endpoint.display());
    store_.set_nmea_link_status(telemetry::LinkStatus::Connecting);

    nmea_tcp_client_.set_line_callback([this](std::string line) { on_nmea_line(std::move(line)); });
    nmea_tcp_client_.set_status_callback([this](bool connected, std::string message) {
        store_.set_nmea_link_status(connected ? telemetry::LinkStatus::Live : telemetry::LinkStatus::Connecting,
                                    std::move(message));
    });
    nmea_tcp_client_.start(std::move(endpoint));
}

void MainWindow::on_nmea_line(std::string line) {
    const auto now = std::chrono::steady_clock::now();
    auto result = nmea::parse_nmea0183_line(line, nmea_parser_state_, now);
    store_.record_nmea_parse_status(result.status, now);
    if (result.status == nmea::ParseStatus::Ok) {
        store_.apply_nmea_update(result.update, "nmea0183");
    }
}

bool MainWindow::on_tick() {
    if (use_dummy_) {
        auto snapshot = dummy_source_.next_snapshot();
        snapshot.sources.nmea0183.status = telemetry::LinkStatus::Disabled;
        snapshot.sources.nmea0183.endpoint = "dummy";
        store_.replace_snapshot(snapshot);
    } else {
        store_.refresh_staleness();
    }
    shell_.update(store_.snapshot());
    return true;
}

} // namespace helm::app
