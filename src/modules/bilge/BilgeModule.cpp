#include "modules/bilge/BilgeModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <string>

namespace helm::modules::bilge {
namespace {

void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& s, double x, double y, double size) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(s);
}

} // namespace

BilgeCanvas::BilgeCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &BilgeCanvas::on_draw));
}

void BilgeCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void BilgeCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);

    graphics::set_source(cr, t.background);
    cr->paint();

    const double left = w * 0.08;
    const double top = h * 0.12;
    const double boat_w = w * 0.46;
    const double boat_h = h * 0.56;

    graphics::set_source(cr, t.panel2);
    cr->move_to(left + boat_w * 0.10, top + boat_h * 0.22);
    cr->line_to(left + boat_w * 0.90, top + boat_h * 0.22);
    cr->line_to(left + boat_w * 0.74, top + boat_h * 0.74);
    cr->line_to(left + boat_w * 0.50, top + boat_h * 0.88);
    cr->line_to(left + boat_w * 0.26, top + boat_h * 0.74);
    cr->close_path();
    cr->fill_preserve();
    graphics::set_source(cr, t.border);
    cr->set_line_width(3.0);
    cr->stroke();

    graphics::set_source(cr, snapshot_.bilge.status == telemetry::BilgeStatus::Dry ? t.ok : t.warning);
    cr->rectangle(left + boat_w * 0.34, top + boat_h * 0.54, boat_w * 0.32, boat_h * 0.14);
    cr->fill();

    graphics::draw_icon(cr, graphics::IconKind::Bilge,
                        snapshot_.bilge.status == telemetry::BilgeStatus::Dry ? graphics::IconState::Ok : graphics::IconState::Warning,
                        left + boat_w * 0.38, top + boat_h * 0.34, boat_w * 0.24, boat_w * 0.24);

    const double px = w * 0.62;
    graphics::set_source(cr, t.text);
    text(cr, "BILGE STATUS", px, 70, 28);
    graphics::set_source(cr, snapshot_.bilge.status == telemetry::BilgeStatus::Dry ? t.ok : t.warning);
    text(cr, telemetry::format::bilge_status(snapshot_.bilge.status), px, 112, 26);

    graphics::set_source(cr, t.muted);
    text(cr, "Pump", px, 178, 14);
    text(cr, "High water", px, 254, 14);
    text(cr, "Current", px, 330, 14);
    text(cr, "Runtime today", px, 406, 14);

    graphics::set_source(cr, t.text);
    text(cr, snapshot_.bilge.pump_on.value ? "ON" : "OFF", px, 214, 34);
    text(cr, snapshot_.bilge.high_water.value ? "WET" : "DRY", px, 290, 34);
    text(cr, telemetry::format::amps(snapshot_.bilge.pump_current_a.value, 1), px, 366, 34);
    text(cr, telemetry::format::fixed(snapshot_.bilge.runtime_today_min.value, 1) + " min", px, 442, 34);
}

BilgeModule::BilgeModule()
    : root_(Gtk::Orientation::VERTICAL, 6), title_("BILGE"), subtitle_("Status first; manual control is timed/hold-to-run only"),
      controls_(Gtk::Orientation::HORIZONTAL, 8) {
    root_.set_margin_top(10);
    root_.set_margin_bottom(10);
    root_.set_margin_start(10);
    root_.set_margin_end(10);
    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    subtitle_.add_css_class("module-subtitle");
    subtitle_.set_xalign(0.0f);

    auto* run = Gtk::make_managed<Gtk::Button>("HOLD TO RUN PUMP");
    auto* timed = Gtk::make_managed<Gtk::Button>("RUN 30 SEC");
    auto* stop = Gtk::make_managed<Gtk::Button>("STOP");
    stop->add_css_class("control-danger");
    for (auto* b : {run, timed, stop}) {
        b->add_css_class("control-button");
        controls_.append(*b);
    }

    root_.append(title_);
    root_.append(subtitle_);
    root_.append(canvas_);
    root_.append(controls_);
}

void BilgeModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::bilge
