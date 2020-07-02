#ifndef __TARGET_H_
#define __TARGET_H_

#include "common.h"

typedef enum {
	TARGET_LED_SER = 0,
	TARGET_LED_ACT,
	TARGET_LED_ERR,
	TARGET_LED_LAST = TARGET_LED_ERR,
	TARGET_RESET,
	TARGET_POWER_ENABLE,
	TARGET_POWER_OC,
	TARGET_POWER_1,
	TARGET_POWER_2
} target_pin;

#define TARGET_POWER_OC_INT	(0)

err_code target_led_set_state(target_pin led, bool state);
void target_reset_set_state(bool state);
void target_reset(void);
void target_unreset(void);
#endif /* __TARGET_H_ */