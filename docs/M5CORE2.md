# M5Stack Core2, experimental

Added in ESP Screen Manager 0.2.133, firmware 0.2.104.
This is the **M5Stack Core2** (and the Core2 for AWS, which has the same screen, touch panel and power chip): a 2-inch
320 x 240 IPS panel with an ILI9342C driver over SPI, FT6336U capacitive touch, an ESP32 with 16 MB flash and 8 MB
quad PSRAM, and an AXP192 power chip. The Core2 **v1.1** has an AXP2101 instead of the AXP192 and does not work with
this board file yet.
Physical acceptance has not been performed.

## Install

Update ESP Screen Manager and choose **M5Stack · 2 inch** (Core2, marked Experimental) in **New screen**.
Follow [Easy setup](EASY_SETUP.md) to create a profile with its own name, Wi-Fi references and unique API/OTA keys.
Choose the correct USB port, or download the firmware to flash with ESPHome Web from your own computer.
Keep an existing working profile if the board is already installed. Pair it through the ESPHome integration before adding tiles.

The remote package is `packages/m5core2.yaml`; the checkout entry is `checkout/m5core2.yaml`.
Both combine `packages/core.yaml` with `packages/boards/m5core2.yaml`.

## Layout and capabilities

- The glass has the CYD's pixels on a smaller diagonal (41 x 30 mm, 200 dpi). At its true density a card of the compact
  look would leave room for two per page, so the board file states the CYD's 143 dpi instead: the same six cells, the
  same compact look, the same pixels as a CYD, each a third smaller on the glass. `inch: 2.0` in `boards.yaml` keeps
  the real size in New screen. Standing up it is one column of four, as the CYD.
- The backlight is the AXP192's DCDC3 supply, driven by `components/axp192`: 0 switches it off, any other level sets it
  between 2.5 and 3.3 V, as M5Stack's own driver does. Brightness, standby, night mode, Sleep, Wake and the alert's
  flashes work as on the Guition. The same component powers the panel (LDO2) and takes the panel and the touch
  controller out of reset (AXP192 GPIO4) before either starts.
- The FT6336U reports pixels, so there is no calibration; the shared touch filter and action guard stay in use
  (`features/capacitive-touch.yaml`). The glass reaches 40 px below the picture, onto the three printed circles.
  `components/ft6336u` keeps that strip from the screen: a contact that starts there never becomes a touch of the
  picture, and one that starts on the picture is let go when it slides down there.
- **A** is the previous page, **C** the next, **B** back to page 1 (closing a card or the settings, as the house in
  the top bar). A press first wakes a dimmed screen. A and C do nothing where an edge swipe does nothing either (an
  open card, the settings, a camera); an alert or the touch test ignores all three.
- Camera tiles, live tile pictures, camera alerts and media covers use the shared PSRAM implementation
  (`features/camera.yaml`, app 0.2.133). The pictures need an ESP Screen Manager that knows this board (0.2.133+):
  an older one serves no pictures to a board it has not heard of.
- The speaker, the microphone, the vibration motor, the motion sensor, the clock chip, the SD card and the battery
  gauge stay unused. The screen runs from USB or its battery; the battery is charged by the AXP192's defaults and not
  reported.

## Hardware references

- [M5Stack Core2 documentation](https://docs.m5stack.com/en/core/core2) and its schematic.
- [ESPHome MIPI SPI](https://esphome.io/components/display/mipi_spi/), model `M5CORE2`: CS GPIO5, DC GPIO15, SPI clock
  GPIO18, data GPIO23, 40 MHz, 18-bit pixels, inverted colours.
- I2C (internal): SDA GPIO21, SCL GPIO22, shared by the AXP192 (0x34) and the FT6336U (0x38). The touch panel's
  interrupt line on GPIO39 is not used: the panel is polled every 20 ms, as on the other boards.
- AXP192 registers used: 0x12 (rails on and off), 0x27 (DCDC3 voltage), 0x28 (LDO2 voltage), 0x95/0x96 (GPIO4 as the
  reset line), 0x82 (ADCs), as in M5Stack's `AXP192.cpp`.
- `tests/test_m5core2.cpp` checks the voltage codes and where a contact of the touch panel belongs.

## What to report while testing

1. Successful boot, pairing and appearance in ESP Screens. A dark screen with the firmware running (logs over USB)
   points at the AXP192 start-up: the log says `The AXP192 does not answer` when it cannot reach it.
2. Correct colours and a stable picture across several page changes and cold starts.
3. Physical taps near each corner, slider drags, edge swipes, and A, B and C, including a swipe that starts on a
   circle and one that ends on it. Touch that lands mirrored or swapped points at the touch transform.
4. The brightness setting across its range (the lowest levels may be very dim), standby after the timeout and the wake
   by touch and by a circle, several times in a row, and night mode.
5. A full page of tiles, opening and closing the settings and detail cards, and the quarter-turn setting.
6. Free internal heap with a full layout, any reset or I2C errors, and how long the screen stayed running.

Do not publish Wi-Fi passwords or API/OTA keys with logs.
