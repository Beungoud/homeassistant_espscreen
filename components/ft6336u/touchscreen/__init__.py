from esphome import automation
import esphome.codegen as cg
from esphome.components import i2c, touchscreen
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_THRESHOLD, CONF_TRIGGER_ID

from .. import ft6336u_ns

DEPENDENCIES = ["i2c"]

CONF_ON_BUTTON = "on_button"

FT6336UTouchscreen = ft6336u_ns.class_(
    "FT6336UTouchscreen", touchscreen.Touchscreen, i2c.I2CDevice
)
ButtonTrigger = ft6336u_ns.class_("ButtonTrigger", automation.Trigger.template(cg.int_))

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(FT6336UTouchscreen),
        # The FT6336U's own default; a higher value asks for a firmer touch.
        cv.Optional(CONF_THRESHOLD, default=22): cv.uint8_t,
        # A press of circle A, B or C below the picture: `button` is 0, 1 or 2.
        cv.Optional(CONF_ON_BUTTON): automation.validate_automation(
            {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ButtonTrigger)}
        ),
    }
).extend(i2c.i2c_device_schema(0x38))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await touchscreen.register_touchscreen(var, config)
    await i2c.register_i2c_device(var, config)
    cg.add(var.set_threshold(config[CONF_THRESHOLD]))
    for conf in config.get(CONF_ON_BUTTON, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [(cg.int_, "button")], conf)
