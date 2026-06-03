#include "modules/windlass/WindlassModule.h"

#include "graphics/MarineIcons.h"
#include "graphics/Theme.h"
#include "telemetry/Format.h"

namespace helm::modules::windlass {
namespace {

void text(const Cairo::RefPtr<Cairo::Context>& cr, const std::string& s, double x, double y, double size) {
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL, Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(size);
    cr->move_to(x, y);
    cr->show_text(s);
}

} // namespace

WindlassCanvas::WindlassCanvas() {
    set_hexpand(true);
    set_vexpand(true);
    set_draw_func(sigc::mem_fun(*this, &WindlassCanvas::on_draw));
}

void WindlassCanvas::set_snapshot(const telemetry::TelemetrySnapshot& snapshot) {
    snapshot_ = snapshot;
    queue_draw();
}

void WindlassCanvas::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    const auto& t = graphics::night_theme();
    const double w = static_cast<double>(width);
    const double h = static_cast<double>(height);
    graphics::set_source(cr, t.background);
    cr->paint();

    const double cx = w * 0.33;
    const double cy = h * 0.48;
    graphics::draw_icon(cr, graphics::IconKind::Windlass, graphics::IconState::Warning,
                        cx - 120, cy - 120, 240, 240);
    graphics::draw_icon(cr, graphics::IconKind::Lock, graphics::IconState::Warning,
                        cx - 54, cy + 84, 108, 108);

    const double px = w * 0.62;
    graphics::set_source(cr, t.text);
    text(cr, "WINDLASS", px, 70, 28);
    graphics::set_source(cr, t.caution);
    text(cr, telemetry::format::windlass_status(snapshot_.windlass.status), px, 112, 28);

    graphics::set_source(cr, t.muted);
    text(cr, "Breaker", px, 178, 14);
    text(cr, "Chain deployed", px, 254, 14);
    text(cr, "Motor current", px, 330, 14);
    text(cr, "Control rule", px, 406, 14);

    graphics::set_source(cr, t.text);
    text(cr, snapshot_.windlass.breaker_on.value ? "ON" : "OFF", px, 214, 34);
    text(cr, telemetry::format::meters(snapshot_.windlass.chain_deployed_m.value, 0), px, 290, 34);
    text(cr, telemetry::format::amps(snapshot_.windlass.motor_current_a.value, 1), px, 366, 34);
    text(cr, "PRESS + HOLD ONLY", px, 442, 26);
}

WindlassModule::WindlassModule()
    : root_(Gtk::Orientation::VERTICAL, 6), title_("WINDLASS"), subtitle_("Locked by default; loss of touch must stop motor"),
      controls_(Gtk::Orientation::HORIZONTAL, 8) {
    root_.set_margin_top(10);
    root_.set_margin_bottom(10);
    root_.set_margin_start(10);
    root_.set_margin_end(10);
    title_.add_css_class("module-title");
    title_.set_xalign(0.0f);
    subtitle_.add_css_class("module-subtitle");
    subtitle_.set_xalign(0.0f);

    auto* arm = Gtk::make_managed<Gtk::Button>("ARM WINDLASS");
    auto* down = Gtk::make_managed<Gtk::Button>("HOLD DOWN");
    auto* up = Gtk::make_managed<Gtk::Button>("HOLD UP");
    auto* stop = Gtk::make_managed<Gtk::Button>("STOP");
    stop->add_css_class("control-danger");
    for (auto* b : {arm, down, up, stop}) {
        b->add_css_class("control-button");
        controls_.append(*b);
    }

    root_.append(title_);
    root_.append(subtitle_);
    root_.append(canvas_);
    root_.append(controls_);
}

void WindlassModule::on_telemetry(const telemetry::TelemetrySnapshot& snapshot) {
    canvas_.set_snapshot(snapshot);
}

} // namespace helm::modules::windlass
