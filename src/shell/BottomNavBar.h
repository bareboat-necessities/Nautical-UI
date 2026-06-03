#pragma once

#include <functional>
#include <gtkmm.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace helm::shell {

class BottomNavBar final : public Gtk::Box {
public:
    BottomNavBar();

    void set_on_selected(std::function<void(std::string)> callback);
    void set_active(const std::string& id);

private:
    void add_button(const std::string& id, const std::string& label);

    std::function<void(std::string)> on_selected_;
    std::unordered_map<std::string, Gtk::Button*> buttons_;
};

} // namespace helm::shell
