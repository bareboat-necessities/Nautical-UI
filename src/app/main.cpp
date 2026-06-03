#include "app/MainWindow.h"

#include <gtkmm.h>

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.mgrushinskiy.helmui");
    return app->make_window_and_run<helm::app::MainWindow>(argc, argv);
}
