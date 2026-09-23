#pragma once
// The AXP192's register values for the Core2's rails, without ESPHome, so tests/test_m5core2.cpp checks them.
#include <cstdint>

namespace esphome::axp192 {

// DCDC1, DCDC2 and DCDC3 take 700-3500 mV in 25 mV steps (datasheet, registers 0x23, 0x26, 0x27).
inline uint8_t dcdc_code(int millivolts) {
  if (millivolts < 700) millivolts = 700;
  if (millivolts > 3500) millivolts = 3500;
  return (uint8_t) ((millivolts - 700) / 25);
}

// LDO2 and LDO3 take 1800-3300 mV in 100 mV steps, LDO2 in the high nibble of register 0x28.
inline uint8_t ldo_code(int millivolts) {
  if (millivolts < 1800) millivolts = 1800;
  if (millivolts > 3300) millivolts = 3300;
  return (uint8_t) ((millivolts - 1800) / 100);
}

// The backlight's LED supply (DCDC3) for a brightness between 0 and 1. Below 2.5 V the LEDs of this panel stay
// dark, so a level above 0 starts there (M5Stack's own driver uses the same 2.5-3.3 V range), and 0 switches
// the rail off. Returns the voltage code, or -1 for off.
inline int backlight_code(float level) {
  if (!(level > 0.0f)) return -1;
  if (level > 1.0f) level = 1.0f;
  return dcdc_code(2500 + (int) (level * 800.0f + 0.5f));
}

}  // namespace esphome::axp192
