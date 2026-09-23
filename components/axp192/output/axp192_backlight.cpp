#include "axp192_backlight.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::axp192 {

static const char *const TAG = "axp192";

// Registers of the AXP192 (datasheet v1.1).
static const uint8_t REG_POWER_OUTPUT = 0x12;  // bit 0 DCDC1, 1 DCDC3, 2 LDO2, 3 LDO3, 4 DCDC2, 6 EXTEN
static const uint8_t REG_DCDC3_VOLTAGE = 0x27;
static const uint8_t REG_LDO23_VOLTAGE = 0x28;
static const uint8_t REG_GPIO34_FUNCTION = 0x95;
static const uint8_t REG_GPIO34_LEVEL = 0x96;
static const uint8_t REG_ADC_ENABLE = 0x82;

static const uint8_t RAIL_DCDC3 = 1 << 1;
static const uint8_t RAIL_LDO2 = 1 << 2;
static const uint8_t GPIO4_HIGH = 1 << 1;

bool Axp192Backlight::update_bits_(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t current;
  if (!this->read_byte(reg, &current))
    return false;
  const uint8_t next = (current & ~mask) | (value & mask);
  return next == current || this->write_byte(reg, next);
}

void Axp192Backlight::setup() {
  // The panel's logic and the SD card run on LDO2 at 3.3 V; LDO3 is the vibration motor and stays as it is.
  bool ok = this->update_bits_(REG_LDO23_VOLTAGE, 0xF0, ldo_code(3300) << 4);
  ok = ok && this->update_bits_(REG_POWER_OUTPUT, RAIL_LDO2, RAIL_LDO2);
  // The backlight is lit from the start (the light's restore_mode puts its own level on it right after).
  ok = ok && this->update_bits_(REG_DCDC3_VOLTAGE, 0x7F, dcdc_code(3000));
  ok = ok && this->update_bits_(REG_POWER_OUTPUT, RAIL_DCDC3, RAIL_DCDC3);
  // GPIO4 as an NMOS open-drain output (M5Stack's own setting), then the reset pulse the panel and the touch
  // controller share.
  ok = ok && this->update_bits_(REG_GPIO34_FUNCTION, 0x8C, 0x84);
  ok = ok && this->update_bits_(REG_GPIO34_LEVEL, GPIO4_HIGH, 0);
  delay(20);
  ok = ok && this->update_bits_(REG_GPIO34_LEVEL, GPIO4_HIGH, GPIO4_HIGH);
  // The battery's voltage and current ADCs, which M5Stack's driver switches on too.
  ok = ok && this->write_byte(REG_ADC_ENABLE, 0xFF);
  if (!ok) {
    ESP_LOGE(TAG, "The AXP192 does not answer: the panel stays dark");
    this->mark_failed();
    return;
  }
  // The panel and the touch controller need a moment out of reset before they are spoken to.
  delay(100);
  this->written_ = dcdc_code(3000);
}

void Axp192Backlight::write_state(float state) {
  if (this->is_failed())
    return;
  const int code = backlight_code(state);
  if (code == this->written_)
    return;
  bool ok = true;
  if (code < 0) {
    ok = this->update_bits_(REG_POWER_OUTPUT, RAIL_DCDC3, 0);
  } else {
    ok = this->update_bits_(REG_DCDC3_VOLTAGE, 0x7F, code);
    if (this->written_ < 0)
      ok = ok && this->update_bits_(REG_POWER_OUTPUT, RAIL_DCDC3, RAIL_DCDC3);
  }
  if (ok)
    this->written_ = code;
  else
    ESP_LOGW(TAG, "Backlight level not written");
}

void Axp192Backlight::dump_config() {
  ESP_LOGCONFIG(TAG, "AXP192 backlight (M5Stack Core2)");
  LOG_I2C_DEVICE(this);
}

}  // namespace esphome::axp192
