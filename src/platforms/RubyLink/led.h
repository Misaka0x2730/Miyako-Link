#ifndef __LED_H
#define __LED_H

#include "general.h"

typedef enum
{
    LED_FIRST = 0,
    LED_ETH = LED_FIRST,
    LED_USB,
    LED_SYS,
    LED_TARGET_1_ACT,
    LED_TARGET_1_SER,
    LED_TARGET_1_ERR,
    LED_TARGET_2_ACT,
    LED_TARGET_2_SER,
    LED_TARGET_2_ERR,
    LED_LAST = LED_TARGET_2_ERR,
    LED_COUNT
} led_id_t;

void led_init(void);
void led_update_enabled(void);
void led_on(const led_id_t id);
void led_off(const led_id_t id);
void led_set_state(const led_id_t id, const bool state);

#endif // __LED_H