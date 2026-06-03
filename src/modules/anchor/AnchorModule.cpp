#include "modules/anchor/AnchorModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <cmath>

namespace helm::modules::anchor {
namespace {
constexpr double pi = 3.14159265358979323846;

void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& s, double x, double y, double size, bool bold = true) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL,
                         bold ? Cairo::ToyFontFace::Weight::BOLD : Cairo::ToyFontFace::Weight::NORMAL);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(s);
}

} // namespace

AnchorCanvas::AnchorCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &AnchorCanvas::on_draw));
}

void AnchorCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void AnchorCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);
    graphics::set_source(cr, t.background);
    cr->paint();

    const double cx = w * 0.38;
    const double cy = h * 0.48;
    const double radius_px = std::min(w, h) * 0.32;
    const double ratio = snapshot_.anchor.radius_m.value > 0.0
                             ? snapshot_.anchor.distance_m.value / snapshot_.anchor.radius_m.value
                             : 0.0;
    const double boat_r = std::min(radius_px * 0.95, radius_px * ratio);

    graphics::set_source(cr, t.panel2);
    cr->arc(cx, cy, radius_px * 1.10, 0, 2 * pi);
    cr->fill();

    graphics::set_source(cr, t.border);
    cr->set_line_width(3.0);
    cr->arc(cx, cy, radius_px, 0, 2 * pi);
    cr->stroke();

    graphics::set_source(cr, t.muted);
    cr->set_line_width(1.0);
    for (int i = 1; i <= 3; ++i) {
        cr->arc(cx, cy, radius_px * i / 4.0, 0, 2 * pi);
        cr->stroke();
    }

    graphics::draw_icon(cr, graphics::IconKind::Anchor, graphics::IconState::Ok, cx - 24, cy - 24, 48, 48);

    const double angle = -35.0 * pi / 180.0;
    const double bx = cx + std::cos(angle) * boat_r;
    const double by = cy + std::sin(angle) * boat_r;
    graphics::draw_icon(cr, graphics::IconKind::Sail, graphics::IconState::Active, bx - 24, by - 24, 48, 48);

    graphics::set_source(cr, t.active);
    cr->set_line_width(2.0);
    cr->move_to(cx, cy);
    cr->line_to(bx, by);
    cr->stroke();

    const double panel_x = w * 0.68;
    graphics::set_source(cr, t.text);
    text(cr, "ANCHOR WATCH", panel_x, 70, 28);
    graphics::set_source(cr, t.ok);
    text(cr, telemetry::format::anchor_status(snapshot_.anchor.status), panel_x, 112, 24);

    graphics::set_source(cr, t.muted);
    text(cr, "Distance", panel_x, 170, 14);
    text(cr, "Radius", panel_x, 245, 14);
    text(cr, "Depth", panel_x, 320, 14);
    text(cr, "Windlass", panel_x, 395, 14);

    graphics::set_source(cr, t.text);
    text(cr, telemetry::format::meters(snapshot_.anchor.distance_m.value, 0), panel_x, 206, 34);
    text(cr, telemetry::format::meters(snapshot_.anchor.radius_m.value, 0), panel_x, 281, 34);
    text(cr, telemetry::format::meters(snapshot_.nav.depth_m.value, 1), panel_x, 356, 34);
    text(cr, telemetry::format::windlass_status(snapshot_.windlass.status), panel_x, 431, 28);
}

AnchorModule::AnchorModule()
    : root_(Gtk::Orientation::VERTICAL, 6), title_("ANCHOR"), subtitle_("Set anchor, monitor swing circle, and keep windlass locked by default"),
      controls_(Gtk::Orientation::HORIZONTAL, 8) {
    root_.set_margin_top(10);
    root_.set_margin_bottom(10);
    root_.set_margin_start(10);
    root_.set_margin_end(10);
    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    subtitle_.add_css_class("module-subtitle");
    subtitle_.set_xalign(0.0f);

    auto* set_anchor = Gtk::make_managed<Gtk::Button>("SET ANCHOR HERE");
    auto* radius = Gtk::make_managed<Gtk::Button>("RADIUS");
    auto* arm = Gtk::make_managed<Gtk::Button>("ARM WATCH");
    auto* disarm = Gtk::make_managed<Gtk::Button>("DISARM");
    for (auto* b : {set_anchor, radius, arm, disarm}) {
        b->add_css_class("control-button");
        controls_.append(*b);
    }

    root_.append(title_);
    root_.append(subtitle_);
    root_.append(canvas_);
    root_.append(controls_);
}

void AnchorModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::anchor
