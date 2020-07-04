#ifndef __TARGET_H_
#define __TARGET_H_

#include "chip.h"
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
	TARGET_POWER_2,
	TARGET_VREF,
	TARGET_CURRENT
} target_pin;

#define TARGET_POWER_OC_INT	(0)
#define TARGET_ADC_SAMPLE_RATE (240000)
#define TARGET_ADC_VREF_CH		(9)
#define TARGET_ADC_CURRENT_CH	(3)

#define TARGET_ANALOG_MAX					(0xFA-1)
#define TARGET_ANALOG_VREF_NUMERATOR		(1289)		//Vref(V) = result * 0.025
#define TARGET_ANALOG_VREF_DENOMINATOR		(5000)
#define TARGET_ANALOG_CURRENT_DENOMINATOR	(4)			//Current(A) = (0.2/256)*result 

void target_init(void);
err_code target_led_set_state(target_pin led, bool state);
void target_reset_set_state(bool state);
void target_reset(void);
void target_unreset(void);
void target_power_enable(void);
void target_power_disable(void);
void target_power_1v8(void);
void target_power_3v3(void);
void target_power_5v(void);
void target_set_over_current_flag(void);
void target_clear_over_current_flag(void);
bool target_get_over_current_flag(void);
uint8_t target_get_vref(void);
uint8_t target_get_current(void);
bool target_get_adc_flag(void);
void target_calculate_vref_and_current(void);
#endif /* __TARGET_H_ */