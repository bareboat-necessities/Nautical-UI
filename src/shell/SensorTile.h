#pragma once

#include "graphics/MarineIcons.h"

#include <gtkmm.h>
#include <string>

namespace helm::shell {

class SensorTile final : public Gtk::Box {
public:
    SensorTile(graphics::IconKind icon, std::string label);

    void set_value(const std::string& value);
    void set_subtitle(const std::string& subtitle);
    void set_state(graphics::IconState state);

private:
    graphics::MarineIconWidget icon_;
    Gtk::Label label_;
    Gtk::Label value_;
    Gtk::Label subtitle_;
};

} // namespace helm::shell
