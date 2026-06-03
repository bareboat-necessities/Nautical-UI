#include "graphics/MarineIcons.h"

#include <algorithm>
#include <cmath>

namespace helm::graphics {
namespace {

constexpr double pi = 3.14159265358979323846;

Color state_color(IconState state, const Theme& t) {
    switch (state) {
        case IconState::Active: return t.active;
        case IconState::Ok: return t.ok;
        case IconState::Warning: return t.caution;
        case IconState::Danger: return t.danger;
        case IconState::Disabled: return Color{t.muted.r, t.muted.g, t.muted.b, 0.45};
        case IconState::Normal: return t.text;
    }
    return t.text;
}

void stroke_round(const Cairo::RefPtr<Cairo::Context>& cr) {
    cr->set_line_cap(Cairo::Context::LineCap::ROUND);
    cr->set_line_join(Cairo::Context::LineJoin::ROUND);
}

void draw_boat_triangle(const Cairo::RefPtr<Cairo::Context>& cr, double cx, double cy, double s) {
    cr->move_to(cx, cy - s);
    cr->line_to(cx + s * 0.58, cy + s * 0.75);
    cr->line_to(cx, cy + s * 0.42);
    cr->line_to(cx - s * 0.58, cy + s * 0.75);
    cr->close_path();
}

void draw_simple_sail(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    cr->move_to(x + w * 0.48, y + h * 0.12);
    cr->line_to(x + w * 0.48, y + h * 0.82);
    cr->line_to(x + w * 0.18, y + h * 0.82);
    cr->line_to(x + w * 0.48, y + h * 0.12);
    cr->move_to(x + w * 0.54, y + h * 0.18);
    cr->line_to(x + w * 0.80, y + h * 0.82);
    cr->line_to(x + w * 0.54, y + h * 0.82);
    cr->close_path();
}

void draw_anchor(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    const double cx = x + w * 0.50;
    const double top = y + h * 0.18;
    const double bottom = y + h * 0.80;
    cr->arc(cx, top, w * 0.09, 0, 2 * pi);
    cr->move_to(cx, top + h * 0.09);
    cr->line_to(cx, bottom);
    cr->move_to(x + w * 0.30, y + h * 0.42);
    cr->line_to(x + w * 0.70, y + h * 0.42);
    cr->move_to(x + w * 0.22, y + h * 0.62);
    cr->curve_to(x + w * 0.28, y + h * 0.82, x + w * 0.42, y + h * 0.88, cx, bottom);
    cr->curve_to(x + w * 0.58, y + h * 0.88, x + w * 0.72, y + h * 0.82, x + w * 0.78, y + h * 0.62);
    cr->move_to(x + w * 0.22, y + h * 0.62);
    cr->line_to(x + w * 0.14, y + h * 0.74);
    cr->move_to(x + w * 0.78, y + h * 0.62);
    cr->line_to(x + w * 0.86, y + h * 0.74);
}

void draw_bilge(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    cr->rectangle(x + w * 0.20, y + h * 0.46, w * 0.60, h * 0.26);
    cr->move_to(x + w * 0.26, y + h * 0.46);
    cr->line_to(x + w * 0.38, y + h * 0.26);
    cr->line_to(x + w * 0.62, y + h * 0.26);
    cr->line_to(x + w * 0.74, y + h * 0.46);
    cr->move_to(x + w * 0.28, y + h * 0.82);
    cr->curve_to(x + w * 0.38, y + h * 0.74, x + w * 0.48, y + h * 0.90, x + w * 0.58, y + h * 0.82);
    cr->curve_to(x + w * 0.66, y + h * 0.76, x + w * 0.72, y + h * 0.82, x + w * 0.80, y + h * 0.78);
}

void draw_windlass(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    cr->rectangle(x + w * 0.20, y + h * 0.42, w * 0.60, h * 0.28);
    cr->arc(x + w * 0.34, y + h * 0.56, w * 0.08, 0, 2 * pi);
    cr->arc(x + w * 0.66, y + h * 0.56, w * 0.08, 0, 2 * pi);
    cr->move_to(x + w * 0.50, y + h * 0.32);
    cr->line_to(x + w * 0.50, y + h * 0.18);
    cr->move_to(x + w * 0.40, y + h * 0.18);
    cr->line_to(x + w * 0.60, y + h * 0.18);
    for (int i = 0; i < 4; ++i) {
        const double cx = x + w * (0.32 + 0.12 * i);
        cr->arc(cx, y + h * 0.82, w * 0.045, 0, 2 * pi);
    }
}

void draw_alarm(const Cairo::RefPtr<Cairo::Context>& cr, double x, double y, double w, double h) {
    cr->move_to(x + w * 0.50, y + h * 0.16);
    cr->line_to(x + w * 0.86, y + h * 0.82);
    cr->line_to(x + w * 0.14, y + h * 0.82);
    cr->close_path();
    cr->move_to(x + w * 0.50, y + h * 0.38);
    cr->line_to(x + w * 0.50, y + h * 0.62);
    cr->move_to(x + w * 0.50, y + h * 0.72);
    cr->line_to(x + w * 0.50, y + h * 0.73);
}

} // namespace

void draw_icon(const Cairo::RefPtr<Cairo::Context>& cr,
               IconKind kind,
               IconState state,
               double x,
               double y,
               double w,
               double h,
               const Theme& theme) {
    const double s = std::min(w, h);
    const double px = x + (w - s) * 0.5;
    const double py = y + (h - s) * 0.5;

    cr->save();
    stroke_round(cr);
    cr->set_line_width(std::max(2.0, s * 0.07));
    set_source(cr, state_color(state, theme));

    switch (kind) {
        case IconKind::Home:
            cr->move_to(px + s * 0.16, py + s * 0.52);
            cr->line_to(px + s * 0.50, py + s * 0.22);
            cr->line_to(px + s * 0.84, py + s * 0.52);
            cr->rectangle(px + s * 0.28, py + s * 0.50, s * 0.44, s * 0.34);
            cr->stroke();
            break;
        case IconKind::Nav:
        case IconKind::Compass:
            cr->arc(px + s * 0.5, py + s * 0.5, s * 0.34, 0, 2 * pi);
            cr->stroke();
            cr->save();
            cr->translate(px + s * 0.5, py + s * 0.5);
            cr->rotate(-25.0 * pi / 180.0);
            draw_boat_triangle(cr, 0, 0, s * 0.26);
            cr->fill();
            cr->restore();
            break;
        case IconKind::Sail:
            draw_simple_sail(cr, px, py, s, s);
            cr->stroke();
            break;
        case IconKind::Anchor:
            draw_anchor(cr, px, py, s, s);
            cr->stroke();
            break;
        case IconKind::Ais:
            draw_boat_triangle(cr, px + s * 0.5, py + s * 0.48, s * 0.24);
            cr->stroke();
            cr->arc(px + s * 0.5, py + s * 0.5, s * 0.42, -0.85, 0.85);
            cr->stroke();
            cr->arc(px + s * 0.5, py + s * 0.5, s * 0.30, -0.65, 0.65);
            cr->stroke();
            break;
        case IconKind::Systems:
            cr->arc(px + s * 0.5, py + s * 0.5, s * 0.20, 0, 2 * pi);
            cr->stroke();
            for (int i = 0; i < 8; ++i) {
                const double a = i * pi / 4.0;
                cr->move_to(px + s * (0.5 + 0.28 * std::cos(a)), py + s * (0.5 + 0.28 * std::sin(a)));
                cr->line_to(px + s * (0.5 + 0.38 * std::cos(a)), py + s * (0.5 + 0.38 * std::sin(a)));
            }
            cr->stroke();
            break;
        case IconKind::Bilge:
            draw_bilge(cr, px, py, s, s);
            cr->stroke();
            break;
        case IconKind::Windlass:
            draw_windlass(cr, px, py, s, s);
            cr->stroke();
            break;
        case IconKind::Alarm:
            draw_alarm(cr, px, py, s, s);
            cr->stroke();
            break;
        case IconKind::Battery:
            cr->rectangle(px + s * 0.18, py + s * 0.36, s * 0.58, s * 0.28);
            cr->rectangle(px + s * 0.78, py + s * 0.44, s * 0.07, s * 0.12);
            cr->stroke();
            break;
        case IconKind::Wind:
            cr->move_to(px + s * 0.16, py + s * 0.34);
            cr->curve_to(px + s * 0.38, py + s * 0.18, px + s * 0.62, py + s * 0.30, px + s * 0.84, py + s * 0.22);
            cr->move_to(px + s * 0.12, py + s * 0.52);
            cr->curve_to(px + s * 0.38, py + s * 0.44, px + s * 0.56, py + s * 0.60, px + s * 0.78, py + s * 0.50);
            cr->move_to(px + s * 0.20, py + s * 0.70);
            cr->curve_to(px + s * 0.40, py + s * 0.62, px + s * 0.54, py + s * 0.76, px + s * 0.68, py + s * 0.68);
            cr->stroke();
            break;
        case IconKind::Depth:
            cr->move_to(px + s * 0.50, py + s * 0.16);
            cr->line_to(px + s * 0.50, py + s * 0.70);
            cr->move_to(px + s * 0.34, py + s * 0.54);
            cr->line_to(px + s * 0.50, py + s * 0.72);
            cr->line_to(px + s * 0.66, py + s * 0.54);
            cr->move_to(px + s * 0.20, py + s * 0.82);
            cr->line_to(px + s * 0.80, py + s * 0.82);
            cr->stroke();
            break;
        case IconKind::Speed:
            cr->arc(px + s * 0.50, py + s * 0.58, s * 0.32, pi, 0);
            cr->move_to(px + s * 0.50, py + s * 0.58);
            cr->line_to(px + s * 0.70, py + s * 0.42);
            cr->stroke();
            break;
        case IconKind::Media:
            cr->move_to(px + s * 0.34, py + s * 0.24);
            cr->line_to(px + s * 0.72, py + s * 0.50);
            cr->line_to(px + s * 0.34, py + s * 0.76);
            cr->close_path();
            cr->fill();
            break;
        case IconKind::Messages:
            cr->rectangle(px + s * 0.16, py + s * 0.24, s * 0.68, s * 0.44);
            cr->move_to(px + s * 0.34, py + s * 0.68);
            cr->line_to(px + s * 0.26, py + s * 0.82);
            cr->line_to(px + s * 0.48, py + s * 0.68);
            cr->stroke();
            break;
        case IconKind::Lock:
            cr->rectangle(px + s * 0.24, py + s * 0.46, s * 0.52, s * 0.36);
            cr->move_to(px + s * 0.34, py + s * 0.46);
            cr->arc(px + s * 0.50, py + s * 0.46, s * 0.16, pi, 2 * pi);
            cr->stroke();
            break;
    }

    cr->restore();
}

MarineIconWidget::MarineIconWidget(IconKind kind, IconState state, int size)
    : kind_(kind), state_(state) {
    set_content_width(size);
    set_content_height(size);
    set_draw_func(sigc::mem_fun(*this, &MarineIconWidget::on_draw));
}

void MarineIconWidget::set_kind(IconKind kind) {
    kind_ = kind;
    queue_draw();
}

void MarineIconWidget::set_state(IconState state) {
    state_ = state;
    queue_draw();
}

void MarineIconWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
    draw_icon(cr, kind_, state_, 0, 0, static_cast<double>(width), static_cast<double>(height));
}

} // namespace helm::graphics
