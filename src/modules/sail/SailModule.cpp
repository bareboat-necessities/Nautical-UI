#include "modules/sail/SailModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace helm::modules::sail {
namespace {
constexpr double pi = 3.14159265358979323846;

template <typename T, typename Formatter>
std::string fmt_or_dash(const telemetry::TimedValue<T>& tv, Formatter formatter) {
    return tv.has_value() ? formatter(tv.value) : "--";
}

double clamp(double v, double lo, double hi) {
    return std::max(lo, std::min(hi, v));
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

SailCanvas::SailCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &SailCanvas::on_draw));
}

void SailCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void SailCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);
    graphics::set_source(cr, t.background);
    cr->paint();

    const double dial_x = 18.0;
    const double dial_y = 18.0;
    const double dial_w = w * 0.56;
    const double dial_h = h - 36.0;
    panel(cr, dial_x, dial_y, dial_w, dial_h);

    const double cx = dial_x + dial_w * 0.50;
    const double cy = dial_y + dial_h * 0.50;
    const double r = std::min(dial_w, dial_h) * 0.37;

    graphics::set_source(cr, t.panel2);
    cr->arc(cx, cy, r * 1.10, 0, 2.0 * pi);
    cr->fill();
    graphics::set_source(cr, t.border);
    cr->set_line_width(2.0);
    cr->arc(cx, cy, r, 0, 2.0 * pi);
    cr->stroke();

    for (int deg = -180; deg <= 180; deg += 15) {
        const double a = (deg - 90.0) * pi / 180.0;
        const double tick = (deg % 45 == 0) ? 0.13 : 0.06;
        graphics::set_source(cr, (deg % 45 == 0) ? t.text : t.muted);
        cr->set_line_width((deg % 45 == 0) ? 2.0 : 1.0);
        cr->move_to(cx + std::cos(a) * r * (1.0 - tick), cy + std::sin(a) * r * (1.0 - tick));
        cr->line_to(cx + std::cos(a) * r, cy + std::sin(a) * r);
        cr->stroke();
    }

    // Upwind target bands.
    graphics::set_source(cr, t.ok);
    cr->set_line_width(6.0);
    cr->arc(cx, cy, r * 0.86, (-50.0 - 90.0) * pi / 180.0, (-32.0 - 90.0) * pi / 180.0);
    cr->stroke();
    cr->arc(cx, cy, r * 0.86, (32.0 - 90.0) * pi / 180.0, (50.0 - 90.0) * pi / 180.0);
    cr->stroke();

    graphics::draw_icon(cr, graphics::IconKind::Sail, graphics::IconState::Active, cx - 42.0, cy - 48.0, 84.0, 96.0);

    const double twa = snapshot_.wind.twa_deg.has_value() ? snapshot_.wind.twa_deg.value : 0.0;
    const double awa = snapshot_.wind.awa_deg.has_value() ? snapshot_.wind.awa_deg.value : twa;
    const double twa_a = (twa - 90.0) * pi / 180.0;
    const double awa_a = (awa - 90.0) * pi / 180.0;

    graphics::set_source(cr, t.active);
    cr->set_line_width(5.0);
    cr->move_to(cx, cy);
    cr->line_to(cx + std::cos(twa_a) * r * 0.94, cy + std::sin(twa_a) * r * 0.94);
    cr->stroke();

    graphics::set_source(cr, t.caution);
    cr->set_line_width(3.0);
    cr->move_to(cx, cy);
    cr->line_to(cx + std::cos(awa_a) * r * 0.78, cy + std::sin(awa_a) * r * 0.78);
    cr->stroke();

    graphics::set_source(cr, t.text);
    text(cr, "WIND / SAIL TRIM", dial_x + 24.0, dial_y + 44.0, 22.0);
    graphics::set_source(cr, t.muted);
    text(cr, "cyan=TWA  yellow=AWA  green bands=upwind target", dial_x + 24.0, dial_y + dial_h - 24.0,
         13.0, Cairo::ToyFontFace::Weight::NORMAL);

    const double card_x = dial_x + dial_w + 18.0;
    const double card_w = w - card_x - 18.0;
    const double card_h = (h - 60.0) / 4.0;
    for (int i = 0; i < 4; ++i) {
        panel(cr, card_x, 18.0 + i * (card_h + 8.0), card_w, card_h);
    }

    const double sog = snapshot_.nav.sog_kt.has_value() ? snapshot_.nav.sog_kt.value : 0.0;
    const double tws = snapshot_.wind.tws_kt.has_value() ? snapshot_.wind.tws_kt.value : 0.0;
    const double target = clamp(0.44 * tws + 0.8, 3.0, 8.5);
    const double perf = target > 0.1 ? clamp(100.0 * sog / target, 0.0, 140.0) : 0.0;
    const double vmg = sog * std::cos(std::abs(twa) * pi / 180.0);

    graphics::set_source(cr, t.muted);
    text(cr, "TRUE WIND", card_x + 18.0, 48.0, 13.0);
    text(cr, "APPARENT WIND", card_x + 18.0, 48.0 + card_h + 8.0, 13.0);
    text(cr, "PERFORMANCE", card_x + 18.0, 48.0 + 2.0 * (card_h + 8.0), 13.0);
    text(cr, "VMG", card_x + 18.0, 48.0 + 3.0 * (card_h + 8.0), 13.0);

    graphics::set_source(cr, t.text);
    text(cr, fmt_or_dash(snapshot_.wind.twa_deg, [](double v) { return telemetry::format::degrees(v, 0); }) + "   " +
              fmt_or_dash(snapshot_.wind.tws_kt, [](double v) { return telemetry::format::knots(v, 1); }),
         card_x + 18.0, 86.0, 28.0);
    text(cr, fmt_or_dash(snapshot_.wind.awa_deg, [](double v) { return telemetry::format::degrees(v, 0); }) + "   " +
              fmt_or_dash(snapshot_.wind.aws_kt, [](double v) { return telemetry::format::knots(v, 1); }),
         card_x + 18.0, 86.0 + card_h + 8.0, 28.0);

    graphics::set_source(cr, perf >= 90.0 ? t.ok : (perf >= 75.0 ? t.caution : t.warning));
    text(cr, telemetry::format::fixed(perf, 0) + "%", card_x + 18.0, 86.0 + 2.0 * (card_h + 8.0), 32.0);
    graphics::set_source(cr, t.muted);
    text(cr, "target " + telemetry::format::knots(target, 1), card_x + 128.0, 86.0 + 2.0 * (card_h + 8.0),
         16.0, Cairo::ToyFontFace::Weight::NORMAL);

    graphics::set_source(cr, t.text);
    text(cr, telemetry::format::knots(vmg, 1), card_x + 18.0, 86.0 + 3.0 * (card_h + 8.0), 32.0);
    graphics::set_source(cr, t.muted);
    text(cr, "computed from SOG and TWA until polar/STW data arrive", card_x + 18.0,
         113.0 + 3.0 * (card_h + 8.0), 12.0, Cairo::ToyFontFace::Weight::NORMAL);
}

SailModule::SailModule()
    : root_(Gtk::Orientation::VERTICAL, 6), title_("SAIL / WIND"),
      subtitle_("Large wind dial, trim target bands, VMG, and simple performance estimate") {
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

void SailModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::sail
