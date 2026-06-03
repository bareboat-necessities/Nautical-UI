#include "modules/home/HomeModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <algorithm>
#include <cmath>

namespace helm::modules::home {
namespace {
constexpr double pi = 3.14159265358979323846;

void draw_text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& text, double x, double y, double size) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(text);
}

void rounded_rect_path(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h, double r) {
    const double rr = std::min({r, w * 0.5, h * 0.5});
    cr->begin_new_sub_path();
    cr->arc(x + w - rr, y + rr, rr, -pi / 2.0, 0.0);
    cr->arc(x + w - rr, y + h - rr, rr, 0.0, pi / 2.0);
    cr->arc(x + rr, y + h - rr, rr, pi / 2.0, pi);
    cr->arc(x + rr, y + rr, rr, pi, 3.0 * pi / 2.0);
    cr->close_path();
}

void draw_panel(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    const auto& t = graphics::night_theme();
    graphics::set_source(cr, t.panel);
    rounded_rect_path(cr, x, y, w, h, 16.0);
    cr->fill_preserve();
    graphics::set_source(cr, t.border);
    cr->set_line_width(1.5);
    cr->stroke();
}

} // namespace

HomeCanvas::HomeCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &HomeCanvas::on_draw));
}

void HomeCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void HomeCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);

    graphics::set_source(cr, t.background);
    cr->paint();

    const double cx = w * 0.50;
    const double cy = h * 0.43;
    const double r = std::min(w, h) * 0.30;

    cr->save();
    graphics::set_source(cr, t.panel2);
    cr->arc(cx, cy, r * 1.08, 0, 2 * pi);
    cr->fill();
    graphics::set_source(cr, t.border);
    cr->set_line_width(2.0);
    cr->arc(cx, cy, r, 0, 2 * pi);
    cr->stroke();

    for (int deg = 0; deg < 360; deg += 10) {
        const double a = (deg - 90.0) * pi / 180.0;
        const double tick = (deg % 30 == 0) ? 0.12 : 0.06;
        const double x1 = cx + std::cos(a) * r * (1.0 - tick);
        const double y1 = cy + std::sin(a) * r * (1.0 - tick);
        const double x2 = cx + std::cos(a) * r;
        const double y2 = cy + std::sin(a) * r;
        graphics::set_source(cr, (deg % 30 == 0) ? t.text : t.muted);
        cr->set_line_width((deg % 30 == 0) ? 2.0 : 1.0);
        cr->move_to(x1, y1);
        cr->line_to(x2, y2);
        cr->stroke();
    }

    const double heading = snapshot_.nav.heading_true_deg.value;
    const double heading_rad = (heading - 90.0) * pi / 180.0;
    graphics::set_source(cr, t.active);
    cr->set_line_width(5.0);
    cr->move_to(cx, cy);
    cr->line_to(cx + std::cos(heading_rad) * r * 0.78, cy + std::sin(heading_rad) * r * 0.78);
    cr->stroke();

    graphics::draw_icon(cr, graphics::IconKind::Sail, graphics::IconState::Ok, cx - 40, cy - 42, 80, 80);

    graphics::set_source(cr, t.text);
    draw_text(cr, "HDG " + telemetry::format::degrees(heading, 0), cx - 82, cy + r + 38, 30);
    graphics::set_source(cr, t.muted);
    draw_text(cr, "COG " + telemetry::format::degrees(snapshot_.nav.cog_deg.value, 0) +
                  "   SOG " + telemetry::format::knots(snapshot_.nav.sog_kt.value, 1),
              cx - 148, cy + r + 70, 18);
    cr->restore();

    const double tile_y = h - 118;
    const double tile_w = (w - 48) / 3.0;
    draw_panel(cr, 12, tile_y, tile_w, 96);
    draw_panel(cr, 24 + tile_w, tile_y, tile_w, 96);
    draw_panel(cr, 36 + tile_w * 2.0, tile_y, tile_w, 96);

    graphics::set_source(cr, t.muted);
    draw_text(cr, "WIND", 30, tile_y + 28, 13);
    draw_text(cr, "ANCHOR", 42 + tile_w, tile_y + 28, 13);
    draw_text(cr, "AIS", 54 + tile_w * 2.0, tile_y + 28, 13);

    graphics::set_source(cr, t.text);
    draw_text(cr, telemetry::format::degrees(snapshot_.wind.twa_deg.value, 0) + "  " + telemetry::format::knots(snapshot_.wind.tws_kt.value, 1),
              30, tile_y + 66, 22);
    draw_text(cr, telemetry::format::meters(snapshot_.anchor.distance_m.value, 0) + " / " + telemetry::format::meters(snapshot_.anchor.radius_m.value, 0),
              42 + tile_w, tile_y + 66, 22);
    draw_text(cr, std::to_string(snapshot_.ais.targets.value) + " targets", 54 + tile_w * 2.0, tile_y + 66, 22);
}

HomeModule::HomeModule() : root_(Gtk::Orientation::VERTICAL, 6), title_("HELM OVERVIEW"), subtitle_("Primary helm overview with compass, wind, anchor, AIS, and live NMEA-ready telemetry") {
    root_.set_margin_top(10);
    root_.set_margin_bottom(10);
    root_.set_margin_start(10);
    root_.set_margin_end(10);
    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    subtitle_.add_css_class("module-subtitle");
    subtitle_.set_xalign(0.0f);
    root_.append(title_);
    root_.append(subtitle_);
    root_.append(canvas_);
}

void HomeModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::home
