#include "shell/BottomNavBar.h"

namespace helm::shell {

BottomNavBar::BottomNavBar() : Gtk::Box(Gtk::Orientation::HORIZONTAL, 2) {
    add_css_class("nav-bar");
    set_margin_start(8);
    set_margin_end(8);
    set_margin_bottom(8);

    add_button("home", "HOME");
    add_button("nav", "NAV");
    add_button("sail", "SAIL");
    add_button("anchor", "ANCHOR");
    add_button("ais", "AIS");
    add_button("systems", "SYSTEMS");
    add_button("alarms", "ALARMS");
}

void BottomNavBar::set_on_selected(std::function<void(std::string)> callback) {
    on_selected_ = std::move(callback);
}

void BottomNavBar::set_active(const std::string& id) {
    for (auto& [button_id, button] : buttons_) {
        if (button_id == id) {
            button->add_css_class("nav-button-active");
        } else {
            button->remove_css_class("nav-button-active");
        }
    }
}

void BottomNavBar::add_button(const std::string& id, const std::string& label) {
    auto* button = Gtk::make_managed<Gtk::Button>(label);
    button->add_css_class("nav-button");
    button->set_hexpand(true);
    button->signal_clicked().connect([this, id] {
        if (on_selected_) {
            on_selected_(id);
        }
    });
    buttons_[id] = button;
    append(*button);
}

} // namespace helm::shell
