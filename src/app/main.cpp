#include "app/AppConfig.h"
#include "app/MainWindow.h"

#include <gtkmm.h>

#include <cstdlib>
#include <string>
#include <vector>

namespace {

helm::app::AppConfig parse_app_config(int& argc, char** argv) {
    helm::app::AppConfig config;
    std::vector<char*> filtered;
    filtered.reserve(static_cast<std::size_t>(argc));
    filtered.push_back(argv[0]);

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg == "--source" && i + 1 < argc) {
            config.nmea_tcp_source = std::string(argv[++i]);
        } else if (arg.rfind("--source=", 0) == 0) {
            config.nmea_tcp_source = arg.substr(9);
        } else if (arg.rfind("--nmea-tcp=", 0) == 0) {
            config.nmea_tcp_source = "tcp-nmea0183://" + arg.substr(11);
        } else if (arg.rfind("tcp-nmea0183://", 0) == 0) {
            config.nmea_tcp_source = arg;
        } else {
            filtered.push_back(argv[i]);
        }
    }

    if (!config.nmea_tcp_source.has_value()) {
        if (const char* env = std::getenv("HELM_UI_NMEA_SOURCE")) {
            if (*env != '\0') {
                config.nmea_tcp_source = std::string(env);
            }
        }
    }

    argc = static_cast<int>(filtered.size());
    for (int i = 0; i < argc; ++i) {
        argv[i] = filtered[static_cast<std::size_t>(i)];
    }
    argv[argc] = nullptr;
    return config;
}

} // namespace

int main(int argc, char* argv[]) {
    auto config = parse_app_config(argc, argv);
    auto app = Gtk::Application::create("com.mgrushinskiy.helmui");
    return app->make_window_and_run<helm::app::MainWindow>(argc, argv, std::move(config));
}
