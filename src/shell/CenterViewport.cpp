#include "shell/CenterViewport.h"

namespace helm::shell {

CenterViewport::CenterViewport() {
    set_hexpand(true);
    set_vexpand(true);
    set_transition_type(Gtk::StackTransitionType::CROSSFADE);
    set_transition_duration(160);
}

void CenterViewport::add_module(app::AppModule& module) {
    add(module.widget(), module.id(), module.title());
}

void CenterViewport::show_module(const std::string& id) {
    active_id_ = id;
    set_visible_child(id);
}

std::string CenterViewport::active_id() const {
    return active_id_;
}

} // namespace helm::shell
