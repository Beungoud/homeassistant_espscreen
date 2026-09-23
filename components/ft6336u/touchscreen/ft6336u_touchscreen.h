#pragma once
#include "touch_strip.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

namespace esphome::ft6336u {

// The Core2's FT6336U, read the way ESPHome's ft63x6 reads it but in one burst per poll, and with the strip below
// the picture kept apart: a point there is never a touch of the screen, a new contact there is `on_button`.
// Polled (update_interval, 20 ms as the other boards): the controller's interrupt line only marks the edges.
class FT6336UTouchscreen : public touchscreen::Touchscreen, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  void set_threshold(uint8_t threshold) { this->threshold_ = threshold; }
  void add_on_button_callback(std::function<void(int)> &&callback) { this->button_callback_.add(std::move(callback)); }

 protected:
  void update_touches() override;
  bool configure_();

  uint8_t threshold_{22};
  bool configured_{false};
  uint32_t attempts_{0};
  ContactOwner owner_;
  CallbackManager<void(int)> button_callback_;
};

class ButtonTrigger : public Trigger<int> {
 public:
  explicit ButtonTrigger(FT6336UTouchscreen *parent) {
    parent->add_on_button_callback([this](int button) { this->trigger(button); });
  }
};

}  // namespace esphome::ft6336u
