#pragma once

#include "graphics/Theme.h"

#include <gtkmm.h>

namespace helm::graphics {

enum class IconKind {
    Home,
    Nav,
    Sail,
    Anchor,
    Ais,
    Systems,
    Bilge,
    Windlass,
    Alarm,
    Battery,
    Wind,
    Depth,
    Speed,
    Compass,
    Media,
    Messages,
    Lock
};

enum class IconState {
    Normal,
    Active,
    Ok,
    Warning,
    Danger,
    Disabled
};

void draw_icon(const Cairo::RefPtr<Cairo::Context>& cr,
               IconKind kind,
               IconState state,
               double x,
               double y,
               double w,
               double h,
               const Theme& theme = night_theme());

class MarineIconWidget final : public Gtk::DrawingArea {
public:
    MarineIconWidget(IconKind kind, IconState state = IconState::Normal, int size = 28);

    void set_kind(IconKind kind);
    void set_state(IconState state);

private:
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

    IconKind kind_;
    IconState state_;
};

} // namespace helm::graphics
