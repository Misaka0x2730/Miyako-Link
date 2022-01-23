#ifndef __SYSTEM_PINS_H
#define __SYSTEM_PINS_H

#include "periph_conf.h"

void system_pins_init(void);
void system_pin_set_value(platform_pin_t pin, bool value);
void system_pin_set(platform_pin_t pin);
void system_pin_clear(platform_pin_t pin);
bool system_pin_get(platform_pin_t pin);

#endif // __SYSTEM_PINS_H