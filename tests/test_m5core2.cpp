#include "../components/axp192/output/axp192_levels.h"
#include "../components/ft6336u/touchscreen/touch_strip.h"
#include <cassert>
int main() {
  using namespace esphome::axp192;
  using namespace esphome::ft6336u;
  // The AXP192's rails: 25 mV steps from 700 mV, LDOs 100 mV steps from 1.8 V, clamped to what the chip takes.
  assert(dcdc_code(700) == 0 && dcdc_code(3300) == 104 && dcdc_code(3500) == 112 && dcdc_code(9000) == 112);
  assert(ldo_code(3300) == 15 && ldo_code(1000) == 0);
  // The backlight: 0 switches DCDC3 off, anything above starts at 2.5 V and 1 is 3.3 V.
  assert(backlight_code(0.0f) == -1 && backlight_code(-1.0f) == -1);
  assert(backlight_code(0.001f) == dcdc_code(2500));
  assert(backlight_code(1.0f) == dcdc_code(3300) && backlight_code(2.0f) == dcdc_code(3300));
  assert(backlight_code(0.5f) == dcdc_code(2900));
  // The strip below the picture: three circles, split where M5Stack splits them.
  assert(strip_button(10, 239) == -1);
  assert(strip_button(0, 240) == 0 && strip_button(106, 279) == 0);
  assert(strip_button(107, 250) == 1 && strip_button(215, 250) == 1);
  assert(strip_button(216, 250) == 2 && strip_button(319, 279) == 2);
  ContactOwner owner;
  bool picture = false;
  // A tap on B presses B once, however long the finger stays, and is never a touch of the picture.
  assert(owner.report(0, 160, 260, picture) == 1 && !picture);
  assert(owner.report(0, 161, 262, picture) == -1 && !picture);
  // Sliding up from B onto the picture stays B's.
  assert(owner.report(0, 161, 200, picture) == -1 && !picture);
  owner.lifted(0);
  assert(owner.report(0, 20, 250, picture) == 0 && !picture);
  owner.lifted(0);
  // A touch of the picture that slides down onto the strip is let go there and never presses a circle.
  assert(owner.report(0, 300, 100, picture) == -1 && picture);
  assert(owner.report(0, 300, 260, picture) == -1 && !picture);
  assert(owner.report(0, 300, 230, picture) == -1 && picture);
  // The second contact is its own.
  assert(owner.report(1, 300, 270, picture) == 2 && !picture);
  owner.lifted(0);
  owner.lifted(1);
  assert(owner.report(1, 50, 50, picture) == -1 && picture);
  return 0;
}
