#include "app/MainWindow.h"

#include "graphics/Theme.h"

#include <glibmm/main.h>

namespace helm::app {

MainWindow::MainWindow() {
    graphics::install_css();
    set_title("Helm UI GTK4");
    set_default_size(1280, 800);
    set_child(shell_);

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::on_tick), 100);
    on_tick();
}

bool MainWindow::on_tick() {
    auto snapshot = dummy_source_.next_snapshot();
    store_.replace_snapshot(snapshot);
    shell_.update(store_.snapshot());
    return true;
}

} // namespace helm::app
