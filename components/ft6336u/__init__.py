"""The FT6336U capacitive touch panel of the M5Stack Core2 (packages/boards/m5core2.yaml).

`touchscreen: platform: ft6336u`: ESPHome's ft63x6, read in one burst, with the glass below the picture kept apart.
The Core2's panel reaches 40 px below it onto three printed circles (A, B, C): a touch there never reaches the
screen and a press of a circle is `on_button` instead.
"""
import esphome.codegen as cg

CODEOWNERS = []

ft6336u_ns = cg.esphome_ns.namespace("ft6336u")
