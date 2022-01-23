#include "hardware/gpio.h"
#include "mux_task.h"
#include "system_pins.h"

void system_pins_init(void)
{
    mux_task_init();
}

void system_pin_set_value(platform_pin_t pin, bool value)
{
    if (PIN_IS_MUX_PIN(pin))
    {
        mux_pin_set(pin, value);
    }
    else if (pin < PIN_NATIVE_COUNT)
    {
        gpio_put(pin, value);
    }
    else
    {
        //TODO: assert
    }
}

void system_pin_set(platform_pin_t pin)
{
    system_pin_set_value(pin, true);
}

void system_pin_clear(platform_pin_t pin)
{
    system_pin_set_value(pin, false);
}

bool system_pin_get(platform_pin_t pin)
{
    if (PIN_IS_MUX_PIN(pin))
    {
        return mux_pin_get(pin);
    }
    else if (pin < PIN_NATIVE_COUNT)
    {
        return gpio_get(pin);
    }
    else
    {
        //TODO: assert
    }
}