#include "modules/ais/AisModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

#include <cmath>
#include <string>

namespace helm::modules::ais {
namespace {
constexpr double pi = 3.14159265358979323846;

void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& s, double x, double y, double size) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(s);
}

} // namespace

AisCanvas::AisCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &AisCanvas::on_draw));
}

void AisCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void AisCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);
    graphics::set_source(cr, t.background);
    cr->paint();

    const double cx = w * 0.38;
    const double cy = h * 0.52;
    const double r = std::min(w, h) * 0.34;

    graphics::set_source(cr, t.panel2);
    cr->arc(cx, cy, r * 1.08, 0, 2 * pi);
    cr->fill();
    graphics::set_source(cr, t.border);
    cr->set_line_width(2.0);
    for (int i = 1; i <= 4; ++i) {
        cr->arc(cx, cy, r * i / 4.0, 0, 2 * pi);
        cr->stroke();
    }
    cr->move_to(cx - r, cy);
    cr->line_to(cx + r, cy);
    cr->move_to(cx, cy - r);
    cr->line_to(cx, cy + r);
    cr->stroke();

    graphics::draw_icon(cr, graphics::IconKind::Sail, graphics::IconState::Active, cx - 26, cy - 26, 52, 52);

    const int targets = std::max(4, snapshot_.ais.targets.value);
    for (int i = 0; i < targets; ++i) {
        const double a = (-90.0 + i * 360.0 / targets + 18.0 * std::sin(i)) * pi / 180.0;
        const double rr = r * (0.20 + 0.70 * (static_cast<double>((i * 37) % 100) / 100.0));
        const bool threat = i == 2 && snapshot_.ais.danger_targets.value > 0;
        graphics::draw_icon(cr, graphics::IconKind::Ais, threat ? graphics::IconState::Danger : graphics::IconState::Ok,
                            cx + std::cos(a) * rr - 16, cy + std::sin(a) * rr - 16, 32, 32);
    }

    const double px = w * 0.68;
    graphics::set_source(cr, t.text);
    text(cr, "AIS TRAFFIC", px, 70, 28);
    graphics::set_source(cr, snapshot_.ais.danger_targets.value > 0 ? t.danger : t.ok);
    text(cr, snapshot_.ais.danger_targets.value > 0 ? "DANGER TARGET" : "CLEAR", px, 112, 24);

    graphics::set_source(cr, t.muted);
    text(cr, "Targets", px, 172, 14);
    text(cr, "Nearest", px, 248, 14);
    text(cr, "Threat CPA", px, 324, 14);
    text(cr, "Threat TCPA", px, 400, 14);

    graphics::set_source(cr, t.text);
    text(cr, std::to_string(snapshot_.ais.targets.value), px, 208, 34);
    text(cr, telemetry::format::nautical_miles(snapshot_.ais.nearest_range_nm.value, 2), px, 284, 34);
    text(cr, telemetry::format::nautical_miles(snapshot_.ais.threat_cpa_nm.value, 2), px, 360, 34);
    text(cr, telemetry::format::fixed(snapshot_.ais.threat_tcpa_min.value, 0) + " min", px, 436, 34);
}

AisModule::AisModule() : root_(Gtk::Orientation::VERTICAL, 6), title_("AIS"), subtitle_("Nearest vessels and threat-ranked CPA/TCPA view placeholder") {
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

void AisModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::ais
