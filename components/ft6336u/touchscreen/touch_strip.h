#pragma once
// Where a contact of the Core2's touch panel lands: on the picture, or on one of the three circles printed below it
// (A, B, C). Without ESPHome, so tests/test_m5core2.cpp checks it.
#include <cstdint>

namespace esphome::ft6336u {

// The panel reports 320 x 280: the picture is y 0-239, the strip of circles y 240-279, and the circles split it at
// x 107 and 216 (M5Stack's own button zones).
static const int PICTURE_HEIGHT = 240;
static const int BUTTON_B_X = 107;
static const int BUTTON_C_X = 216;

// -1 for a point on the picture, 0, 1 or 2 for circle A, B or C.
inline int strip_button(int x, int y) {
  if (y < PICTURE_HEIGHT) return -1;
  if (x < BUTTON_B_X) return 0;
  if (x < BUTTON_C_X) return 1;
  return 2;
}

// Each contact the panel tracks (it tracks two) belongs to where it started for as long as it lasts: a finger that
// starts on a circle and slides up onto the picture never becomes a touch of the picture, and one that starts on the
// picture and slides down is let go there, never a press of a circle.
class ContactOwner {
 public:
  enum Place : int8_t { NONE = -2, PICTURE = -1 };
  // A report of contact `id` at (x, y). Returns the circle pressed by this report (0-2, once per contact), or -1.
  // `on_picture` says whether the point belongs to the picture.
  int report(uint8_t id, int x, int y, bool &on_picture) {
    id &= 1;
    const int here = strip_button(x, y);
    int pressed = -1;
    if (this->place_[id] == NONE) {
      this->place_[id] = here < 0 ? PICTURE : here;
      pressed = here;
    }
    on_picture = this->place_[id] == PICTURE && here < 0;
    return pressed;
  }
  // The panel no longer reports contact `id`.
  void lifted(uint8_t id) { this->place_[id & 1] = NONE; }

 private:
  int8_t place_[2]{NONE, NONE};
};

}  // namespace esphome::ft6336u
