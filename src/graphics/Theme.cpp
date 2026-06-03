#include "graphics/Theme.h"

#include <gdkmm/display.h>

namespace helm::graphics {

const Theme& night_theme() {
    static const Theme theme{};
    return theme;
}

void set_source(const Cairo::RefPtr<Cairo::Context>& cr, Color c) {
    cr->set_source_rgba(c.r, c.g, c.b, c.a);
}

void install_css() {
    auto provider = Gtk::CssProvider::create();
    provider->load_from_data(R"CSS(
window {
  background: #06121c;
  color: #f2f7ff;
  font-family: Sans;
}
.helm-shell {
  background: #06121c;
}
.status-bar, .nav-bar {
  background: #071725;
  border-color: #2f5f7f;
}
.status-chip {
  padding: 5px 10px;
  border-radius: 12px;
  background: #10283a;
  border: 1px solid #2f5f7f;
  color: #f2f7ff;
  font-weight: 700;
}
.status-chip-ok {
  color: #5ee06f;
}
.status-chip-warning {
  color: #ffd34a;
}
.status-chip-danger {
  color: #ff3b30;
}
.sensor-tile {
  background: #0a1b28;
  border: 1px solid #2f5f7f;
  border-radius: 14px;
  padding: 8px;
}
.sensor-label {
  color: #91a9b8;
  font-size: 12px;
  font-weight: 700;
  letter-spacing: 0.08em;
}
.sensor-value {
  color: #f2f7ff;
  font-size: 24px;
  font-weight: 800;
}
.sensor-subtitle {
  color: #91a9b8;
  font-size: 12px;
}
.module-title {
  color: #f2f7ff;
  font-size: 22px;
  font-weight: 800;
}
.module-subtitle {
  color: #91a9b8;
  font-size: 13px;
}
.nav-button {
  margin: 4px;
  padding: 10px 14px;
  border-radius: 14px;
  background: #10283a;
  border: 1px solid #2f5f7f;
  color: #f2f7ff;
  font-weight: 800;
}
.nav-button:hover {
  background: #14334c;
}
.nav-button-active {
  background: #1f7cff;
  border-color: #59b8ff;
}
.control-button {
  padding: 12px 18px;
  border-radius: 14px;
  font-weight: 900;
}
.control-danger {
  background: #8d1f1f;
  color: #ffffff;
}

.control-warning {
  background: #5b4514;
  color: #ffffff;
}
.alarm-card {
  padding: 12px;
  border-radius: 16px;
  background: #0a1b28;
  border: 1px solid #2f5f7f;
}
.alarm-critical {
  border-color: #ff3b30;
  background: #2a0d13;
}
.alarm-warning {
  border-color: #ffd34a;
  background: #241d0a;
}
.alarm-info {
  border-color: #2f5f7f;
  background: #0a1b28;
}
.alarm-severity {
  color: #ffd34a;
  font-size: 16px;
  font-weight: 900;
}
.alarm-title {
  color: #f2f7ff;
  font-size: 20px;
  font-weight: 900;
}
.alarm-detail {
  color: #91a9b8;
  font-size: 14px;
}
)CSS");

    auto display = Gdk::Display::get_default();
    if (display) {
        Gtk::StyleContext::add_provider_for_display(display, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

} // namespace helm::graphics
