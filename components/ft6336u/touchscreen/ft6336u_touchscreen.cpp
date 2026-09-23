#include "ft6336u_touchscreen.h"

#include "esphome/core/log.h"

namespace esphome::ft6336u {

static const char *const TAG = "ft6336u";

// Registers of the FT6336U (FocalTech FT6x36 datasheet).
static const uint8_t REG_DEVICE_MODE = 0x00;
static const uint8_t REG_TD_STATUS = 0x02;  // then 6 bytes per contact: XH, XL, YH, YL, weight, misc
static const uint8_t REG_THRESHOLD = 0x80;
static const uint8_t REG_TOUCHRATE_ACTIVE = 0x88;
static const uint8_t REG_CHIP_ID = 0xA3;

void FT6336UTouchscreen::setup() {
  // The panel's own pixels on the picture; the strip below it (y 240-279) never reaches the scaling.
  if (this->x_raw_max_ == this->x_raw_min_)
    this->x_raw_max_ = this->display_->get_native_width();
  if (this->y_raw_max_ == this->y_raw_min_)
    this->y_raw_max_ = this->display_->get_native_height();
  this->configure_();
}

// The controller comes out of the reset the panel shares with it (AXP192 GPIO4) some hundreds of milliseconds after
// the AXP192 lets go, later than this component starts on some boards: the first attempt then finds it silent. It is
// asked again on every poll until it answers, instead of giving the touch panel up for the whole run.
bool FT6336UTouchscreen::configure_() {
  uint8_t chip_id = 0;
  if (!this->read_byte(REG_CHIP_ID, &chip_id) || chip_id == 0) {
    if (this->attempts_++ == 0)
      ESP_LOGW(TAG, "The FT6336U does not answer yet; asking again on every poll");
    else if (this->attempts_ % 250 == 0)
      ESP_LOGE(TAG, "The FT6336U still does not answer after %u polls", (unsigned) this->attempts_);
    return false;
  }
  this->write_byte(REG_DEVICE_MODE, 0x00);
  this->write_byte(REG_THRESHOLD, this->threshold_);
  this->write_byte(REG_TOUCHRATE_ACTIVE, 0x0E);
  this->configured_ = true;
  ESP_LOGI(TAG, "FT6336U ready (chip id 0x%02X) after %u failed attempt(s)", chip_id, (unsigned) this->attempts_);
  return true;
}

void FT6336UTouchscreen::update_touches() {
  if (!this->configured_ && !this->configure_()) {
    this->skip_update_ = true;
    return;
  }
  uint8_t data[13];
  if (this->read_register(REG_TD_STATUS, data, sizeof(data)) != i2c::ERROR_OK) {
    this->skip_update_ = true;
    return;
  }
  const uint8_t count = data[0] & 0x0F;
  bool seen[2]{false, false};
  for (uint8_t point = 0; point < 2 && point < count; point++) {
    const uint8_t *p = data + 1 + point * 6;
    // The event flag (0 press, 1 lift, 2 contact, 3 none): a lift or an empty slot is not a finger.
    if ((p[0] >> 6) & 0x01)
      continue;
    const int x = ((p[0] & 0x0F) << 8) | p[1];
    const int y = ((p[2] & 0x0F) << 8) | p[3];
    const uint8_t id = (p[2] >> 4) & 0x01;
    seen[id] = true;
    bool on_picture = false;
    const int button = this->owner_.report(id, x, y, on_picture);
    if (button >= 0) {
      ESP_LOGD(TAG, "Button %c", 'A' + button);
      this->button_callback_.call(button);
    }
    if (on_picture)
      this->add_raw_touch_position_(id, x, y, p[4]);
  }
  for (uint8_t id = 0; id < 2; id++)
    if (!seen[id])
      this->owner_.lifted(id);
}

void FT6336UTouchscreen::dump_config() {
  ESP_LOGCONFIG(TAG,
                "M5Stack Core2 touch panel (FT6336U):\n"
                "  Threshold: %u\n"
                "  Buttons A, B, C below the picture",
                this->threshold_);
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
}

}  // namespace esphome::ft6336u
