#include "led.h"
#include "system_pins.h"
#include "system_func.h"
#include "device_settings.h"

static const platform_pin_t led_pin_list[LED_COUNT] = {PIN_LED_ETH, PIN_LED_USB, PIN_LED_SYS,
                                                PIN_TARGET_1_LED_ACT, PIN_TARGET_1_LED_SER, PIN_TARGET_1_LED_ERR,
                                                PIN_TARGET_2_LED_ACT, PIN_TARGET_2_LED_SER, PIN_TARGET_2_LED_ERR};

static uint8_t led_enabled[4] = { 0 };

void led_init(void)
{
    led_update_enabled();
}

void led_update_enabled(void)
{
    device_settings_get_value(DEVICE_SETTINGS_LEDS_ENABLED, led_enabled, 4);

    for (uint32_t i = 0; i < sizeof(led_enabled); i++)
    {
        const uint8_t data = led_enabled[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (!(data & (1 << j)))
            {
                led_off(i*8 + j);
            }
        }
    }
}

void led_on(const led_id_t id)
{
    if (id < LED_COUNT)
    {
        const bool enabled = (led_enabled[id / 8] & (1 << (id % 8))) != 0;

        if (enabled)
        {
            system_pin_set(led_pin_list[id]);
        }
        else
        {
            system_pin_clear(led_pin_list[id]);
        }
    }
}

void led_off(const led_id_t id)
{
    if (id < LED_COUNT)
    {
        system_pin_clear(led_pin_list[id]);
    }
}

void led_set_state(const led_id_t id, const bool state)
{
    if (id < LED_COUNT)
    {
        if (state)
        {
            led_on(id);
        }
        else
        {
            led_off(id);
        }
    }
}