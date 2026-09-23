"""The AXP192 power chip of the M5Stack Core2, as the output that lights its backlight (packages/boards/m5core2.yaml).

`output: platform: axp192`: on the Core2 the backlight is a supply of the AXP192 (DCDC3), not a PWM pin. The same
output powers the panel up and takes it out of reset at boot, before the display and the touch panel start.
"""
import esphome.codegen as cg

CODEOWNERS = []

axp192_ns = cg.esphome_ns.namespace("axp192")
