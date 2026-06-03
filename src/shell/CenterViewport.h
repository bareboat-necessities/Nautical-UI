#pragma once

#include "app/AppModule.h"

#include <gtkmm.h>
#include <string>

namespace helm::shell {

class CenterViewport final : public Gtk::Stack {
public:
    CenterViewport();

    void add_module(app::AppModule& module);
    void show_module(const std::string& id);
    [[nodiscard]] std::string active_id() const;

private:
    std::string active_id_{"home"};
};

} // namespace helm::shell
