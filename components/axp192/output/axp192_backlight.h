#pragma once
#include "axp192_levels.h"

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"

namespace esphome::axp192 {

// The Core2's backlight, and the power-up the panel needs before it can start: the AXP192 feeds the panel's logic
// (LDO2) and its LEDs (DCDC3), and holds the panel and the touch controller in reset on its GPIO4.
class Axp192Backlight : public output::FloatOutput, public Component, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  // After the I2C bus (BUS), before the display and the touch panel (DATA), which need the power it switches on.
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  void write_state(float state) override;
  bool update_bits_(uint8_t reg, uint8_t mask, uint8_t value);

  // The code last written, so a light transition that asks for the same step on every frame writes nothing: the
  // touch panel shares this bus and is read every 20 ms.
  int written_{-2};
};

}  // namespace esphome::axp192
