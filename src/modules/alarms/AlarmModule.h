#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>
#include <vector>

namespace helm::modules::alarms {

class AlarmModule final : public app::AppModule {
public:
    AlarmModule();

    std::string id() const override { return "alarms"; }
    std::string title() const override { return "Alarms"; }
    Gtk::Widget& widget() override { return root_; }
    void on_telemetry(const telemetry::TelemetrySnapshot& snapshot) override;

private:
    struct AlarmRow {
        Gtk::Label severity;
        Gtk::Label title;
        Gtk::Label detail;
    };

    void rebuild_rows(const telemetry::TelemetrySnapshot& snapshot);
    void append_alarm(const std::string& severity, const std::string& title, const std::string& detail, const char* css_class);
    void clear_rows();

    Gtk::Box root_;
    Gtk::Label title_;
    Gtk::Label subtitle_;
    Gtk::Box list_;
    Gtk::Box controls_;
    std::vector<Gtk::Box*> row_widgets_;
};

} // namespace helm::modules::alarms
