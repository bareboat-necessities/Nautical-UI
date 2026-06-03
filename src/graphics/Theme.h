#pragma once

#include <cstdint>
#include <gtkmm.h>

namespace helm::graphics {

struct Color {
    double r{0.0};
    double g{0.0};
    double b{0.0};
    double a{1.0};
};

struct Theme {
    Color background{0.023, 0.071, 0.110, 1.0};
    Color panel{0.039, 0.106, 0.157, 1.0};
    Color panel2{0.063, 0.157, 0.227, 1.0};
    Color border{0.184, 0.373, 0.498, 1.0};
    Color text{0.949, 0.969, 1.000, 1.0};
    Color muted{0.569, 0.663, 0.722, 1.0};
    Color ok{0.369, 0.878, 0.435, 1.0};
    Color caution{1.000, 0.827, 0.290, 1.0};
    Color warning{1.000, 0.541, 0.165, 1.0};
    Color danger{1.000, 0.231, 0.188, 1.0};
    Color active{0.122, 0.612, 1.000, 1.0};
};

const Theme& night_theme();
void set_source(const Cairo::RefPtr<Cairo::Context>& cr, Color c);
void install_css();

} // namespace helm::graphics
