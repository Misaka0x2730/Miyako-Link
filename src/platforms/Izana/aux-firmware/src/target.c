#include "target.h"

static uint32_t target_pins[] = {
	10,
	4,
	11,
	13,
	1,
	0,
	15,
	12
};

void target_init(void)
{
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_SER]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_ACT]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_ERR]);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_RESET]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_ENABLE]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_1]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_2]);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_OC]);

	Chip_SYSCTL_SetPinInterrupt(TARGET_POWER_OC_INT, target_pins[TARGET_POWER_OC]);
	LPC_PININT->ISEL |= PININTCH(TARGET_POWER_OC_INT);	//Set mode = level
	LPC_PININT->CIENF |= PININTCH(TARGET_POWER_OC_INT);	//Set level = low
	LPC_PININT->SIENR |= PININTCH(TARGET_POWER_OC_INT);	//Enable level interrupt
	NVIC_EnableIRQ(PININT0_IRQn);


}

err_code target_led_set_state(target_pin led, bool state)
{
	if(led > TARGET_LED_LAST)
		return ERR_OK;

	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[led], state);
	return ERR_INV_ARG;
}

/* 0 - start target, 1 - RESET target */
void target_reset_set_state(bool state)
{
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_RESET], state);
}

void target_reset(void)
{
	target_reset_set_state(true);
}

void target_unreset(void)
{
	target_reset_set_state(false);
}

void PININT0_IRQHandler(void)
{
	while(1) {

	}
}
