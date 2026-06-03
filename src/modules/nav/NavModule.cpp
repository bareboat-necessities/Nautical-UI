#include "modules/nav/NavModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace helm::modules::nav {
namespace {
constexpr double pi = 3.14159265358979323846;

template <typename T, typename Formatter>
std::string fmt_or_dash(const telemetry::TimedValue<T>& tv, Formatter formatter) {
    return tv.has_value() ? formatter(tv.value) : "--";
}

void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& s, double x, double y, double size,
          Cairo::ToyFontFace::Weight weight = Cairo::ToyFontFace::Weight::BOLD) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, weight);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(s);
}

void rounded_rect(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h, double r) {
    const double rr = std::min({r, w * 0.5, h * 0.5});
    cr->new_sub_path();
    cr->arc(x + w - rr, y + rr, rr, -pi / 2.0, 0.0);
    cr->arc(x + w - rr, y + h - rr, rr, 0.0, pi / 2.0);
    cr->arc(x + rr, y + h - rr, rr, pi / 2.0, pi);
    cr->arc(x + rr, y + rr, rr, pi, 3.0 * pi / 2.0);
    cr->close_path();
}

void panel(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    const auto& t = graphics::night_theme();
    graphics::set_source(cr, t.panel);
    rounded_rect(cr, x, y, w, h, 16.0);
    cr->fill_preserve();
    graphics::set_source(cr, t.border);
    cr->set_line_width(1.5);
    cr->stroke();
}

} // namespace

NavCanvas::NavCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &NavCanvas::on_draw));
}

void NavCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void NavCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);
    graphics::set_source(cr, t.background);
    cr->paint();

    const double map_x = 18.0;
    const double map_y = 18.0;
    const double map_w = w * 0.60;
    const double map_h = h - 36.0;
    panel(cr, map_x, map_y, map_w, map_h);

    graphics::set_source(cr, t.panel2);
    rounded_rect(cr, map_x + 16.0, map_y + 16.0, map_w - 32.0, map_h - 32.0, 14.0);
    cr->fill();

    // Chart-like grid and route. This is a Phase 4 tactical placeholder, not chart rendering yet.
    graphics::set_source(cr, t.border);
    cr->set_line_width(1.0);
    for (int i = 1; i < 5; ++i) {
        const double gx = map_x + 16.0 + (map_w - 32.0) * i / 5.0;
        const double gy = map_y + 16.0 + (map_h - 32.0) * i / 5.0;
        cr->move_to(gx, map_y + 16.0);
        cr->line_to(gx, map_y + map_h - 16.0);
        cr->move_to(map_x + 16.0, gy);
        cr->line_to(map_x + map_w - 16.0, gy);
    }
    cr->stroke();

    const double bx = map_x + map_w * 0.44;
    const double by = map_y + map_h * 0.62;
    const double wx = map_x + map_w * 0.73;
    const double wy = map_y + map_h * 0.30;

    graphics::set_source(cr, t.active);
    cr->set_line_width(4.0);
    cr->move_to(bx, by);
    cr->line_to(wx, wy);
    cr->stroke();

    graphics::set_source(cr, t.caution);
    cr->set_line_width(2.0);
    cr->set_dash(std::vector<double>{9.0, 7.0}, 0.0);
    cr->move_to(bx - 80.0, by + 40.0);
    cr->line_to(wx + 70.0, wy - 20.0);
    cr->stroke();
    cr->unset_dash();

    graphics::draw_icon(cr, graphics::IconKind::Sail, graphics::IconState::Active, bx - 28.0, by - 28.0, 56.0, 56.0);
    graphics::draw_icon(cr, graphics::IconKind::Nav, graphics::IconState::Ok, wx - 22.0, wy - 22.0, 44.0, 44.0);

    graphics::set_source(cr, t.text);
    text(cr, "ROUTE / WAYPOINT", map_x + 30.0, map_y + 48.0, 22.0);
    graphics::set_source(cr, t.muted);
    text(cr, "Chart layer comes in Phase 8. This view already consumes live GPS/COG/SOG/depth.",
         map_x + 30.0, map_y + map_h - 28.0, 13.0, Cairo::ToyFontFace::Weight::NORMAL);

    const double side_x = map_x + map_w + 18.0;
    const double side_w = w - side_x - 18.0;
    const double card_h = (h - 54.0) / 4.0;
    for (int i = 0; i < 4; ++i) {
        panel(cr, side_x, 18.0 + i * (card_h + 6.0), side_w, card_h);
    }

    const double lat = snapshot_.nav.latitude_deg.value;
    const double lon = snapshot_.nav.longitude_deg.value;
    graphics::set_source(cr, t.muted);
    text(cr, "POSITION", side_x + 18.0, 48.0, 13.0);
    text(cr, "COURSE", side_x + 18.0, 48.0 + card_h + 6.0, 13.0);
    text(cr, "DEPTH", side_x + 18.0, 48.0 + 2.0 * (card_h + 6.0), 13.0);
    text(cr, "WAYPOINT", side_x + 18.0, 48.0 + 3.0 * (card_h + 6.0), 13.0);

    graphics::set_source(cr, t.text);
    const std::string pos = fmt_or_dash(snapshot_.nav.latitude_deg, [](double v) { return telemetry::format::fixed(v, 5); }) +
                            " / " + fmt_or_dash(snapshot_.nav.longitude_deg, [](double v) { return telemetry::format::fixed(v, 5); });
    text(cr, snapshot_.nav.latitude_deg.has_value() && snapshot_.nav.longitude_deg.has_value() ? pos : "-- / --",
         side_x + 18.0, 86.0, 24.0);
    (void)lat;
    (void)lon;

    text(cr, "COG " + fmt_or_dash(snapshot_.nav.cog_deg, [](double v) { return telemetry::format::degrees(v, 0); }) +
              "   SOG " + fmt_or_dash(snapshot_.nav.sog_kt, [](double v) { return telemetry::format::knots(v, 1); }),
         side_x + 18.0, 86.0 + card_h + 6.0, 25.0);
    text(cr, fmt_or_dash(snapshot_.nav.depth_m, [](double v) { return telemetry::format::meters(v, 1); }),
         side_x + 18.0, 86.0 + 2.0 * (card_h + 6.0), 30.0);
    text(cr, "NO ACTIVE ROUTE", side_x + 18.0, 86.0 + 3.0 * (card_h + 6.0), 24.0);
    graphics::set_source(cr, t.muted);
    text(cr, "BTW/DTW/XTE parser hooks are reserved for RMB/APB/XTE.",
         side_x + 18.0, 112.0 + 3.0 * (card_h + 6.0), 12.0, Cairo::ToyFontFace::Weight::NORMAL);
}

NavModule::NavModule()
    : root_(Gtk::Orientation::VERTICAL, 6), title_("NAVIGATION"),
      subtitle_("Route/waypoint tactical view with live GPS, COG/SOG, and depth") {
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

void NavModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::nav
