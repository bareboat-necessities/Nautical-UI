#include "shell/SensorTile.h"

namespace helm::shell {

SensorTile::SensorTile(graphics::IconKind icon, std::string label)
    : Gtk::Box(Gtk::Orientation::VERTICAL, 4), icon_(icon, graphics::IconState::Normal, 28), label_(std::move(label)) {
    add_css_class("sensor-tile");
    set_size_request(150, 94);

    auto* row = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 6);
    row->append(icon_);
    label_.add_css_class("sensor-label");
    label_.set_xalign(0.0f);
    row->append(label_);

    value_.add_css_class("sensor-value");
    value_.set_xalign(0.0f);
    value_.set_text("--");

    subtitle_.add_css_class("sensor-subtitle");
    subtitle_.set_xalign(0.0f);
    subtitle_.set_text("waiting");

    append(*row);
    append(value_);
    append(subtitle_);
}

void SensorTile::set_value(const std::string& value) {
    value_.set_text(value);
}

void SensorTile::set_subtitle(const std::string& subtitle) {
    subtitle_.set_text(subtitle);
}

void SensorTile::set_state(graphics::IconState state) {
    icon_.set_state(state);
}

} // namespace helm::shell
