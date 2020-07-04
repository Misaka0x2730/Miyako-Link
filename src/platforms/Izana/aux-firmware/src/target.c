#include "target.h"

static uint32_t target_pins[] = {
	10,
	4,
	11,
	13,
	1,
	0,
	15,
	12,
	17,
	23
};

static const PINMUX_GRP_T muxArray[] = {
	{IOCON_PIO10, PIN_I2CMODE_GPIO},
	{IOCON_PIO4, PIN_MODE_INACTIVE},
	{IOCON_PIO11, PIN_I2CMODE_GPIO},
	{IOCON_PIO13, PIN_MODE_INACTIVE},
	{IOCON_PIO1, PIN_MODE_INACTIVE},
	{IOCON_PIO0, PIN_MODE_PULLUP | PIN_SMODE_CYC3},
	{IOCON_PIO15, PIN_MODE_INACTIVE},
	{IOCON_PIO12, PIN_MODE_INACTIVE}
};

volatile static bool target_over_current_flag = false;
volatile static bool target_adc_flag = false;
volatile static uint8_t vref = 0, current = 0;
volatile static uint16_t vref_raw = 0, current_raw = 0;

void target_init(void)
{
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC3);
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC9);

	Chip_IOCON_SetPinMuxing(LPC_IOCON, muxArray, COUNT_OF(muxArray));

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_SER]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_ACT]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_LED_ERR]);

	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_RESET]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_ENABLE]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_1]);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_2]);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_OC]);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, target_pins[TARGET_VREF]);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, target_pins[TARGET_CURRENT]);

	Chip_SYSCTL_SetPinInterrupt(TARGET_POWER_OC_INT, target_pins[TARGET_POWER_OC]);
	LPC_PININT->ISEL |= PININTCH(TARGET_POWER_OC_INT);	//Set mode = level
	LPC_PININT->CIENF |= PININTCH(TARGET_POWER_OC_INT);	//Set level = low
	LPC_PININT->SIENR |= PININTCH(TARGET_POWER_OC_INT);	//Enable level interrupt
	NVIC_EnableIRQ(PININT0_IRQn);

	Chip_ADC_Init(LPC_ADC, ADC_CR_MODE10BIT);
	Chip_ADC_StartCalibration(LPC_ADC);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC)))
		;
	Chip_ADC_SetClockRate(LPC_ADC, TARGET_ADC_SAMPLE_RATE);

	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQA_IDX,
							(ADC_SEQ_CTRL_CHANSEL(TARGET_ADC_VREF_CH) | ADC_SEQ_CTRL_HWTRIG_POLPOS));
	Chip_ADC_SetupSequencer(LPC_ADC, ADC_SEQB_IDX,
							(ADC_SEQ_CTRL_CHANSEL(TARGET_ADC_CURRENT_CH) | ADC_SEQ_CTRL_HWTRIG_POLPOS));

	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQA_IDX);
	Chip_ADC_EnableSequencer(LPC_ADC, ADC_SEQB_IDX);

	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQB_IDX);
}

err_code target_led_set_state(target_pin led, bool state)
{
	if(led > TARGET_LED_LAST)
		return ERR_OK;

	if(led != TARGET_LED_ACT)
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[led], !state);
	else
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

void target_power_enable(void)
{
	if(!target_get_over_current_flag())
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_ENABLE], true);
}

void target_power_disable(void)
{
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_ENABLE], false);
}

void target_power_1v8(void)
{
	if(!target_get_over_current_flag()) {
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_1], false);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_2], false);
	}
}

void target_power_3v3(void)
{
	if(!target_get_over_current_flag()) {
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_1], true);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_2], false);
	}
}

void target_power_5v(void)
{
	if(!target_get_over_current_flag()) {
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_1], false);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, 0, target_pins[TARGET_POWER_2], true);
	}
}

void target_set_over_current_flag(void)
{
	target_over_current_flag = true;
}

void target_clear_over_current_flag(void)
{
	target_over_current_flag = false;
}

bool target_get_over_current_flag(void)
{
	return target_over_current_flag;
}

uint8_t target_get_vref(void)
{
	return vref;
}

uint8_t target_get_current(void)
{
	return current;
}

bool target_get_adc_flag(void)
{
	bool result = false;

	uint32_t data = Chip_ADC_GetSequencerDataReg(LPC_ADC, ADC_SEQA_IDX);
	if(data & ADC_SEQ_GDAT_DATAVALID) {
		result = true;
		vref_raw = (data >> ADC_SEQ_GDAT_RESULT_BITPOS);
	}

	data = Chip_ADC_GetSequencerDataReg(LPC_ADC, ADC_SEQB_IDX);
	if(data & ADC_SEQ_GDAT_DATAVALID) {
		current_raw = (data >> ADC_SEQ_GDAT_RESULT_BITPOS);
	} else {
		result = false;
	}

	return result;
}

void target_calculate_vref_and_current(void)
{
	uint32_t temp = 0;
	temp = (uint32_t)(vref_raw * TARGET_ANALOG_VREF_NUMERATOR);
	temp = (uint32_t)(temp / TARGET_ANALOG_VREF_DENOMINATOR);
	vref = (uint8_t)(temp & 0xFF);

	temp = (uint32_t)(current_raw / TARGET_ANALOG_CURRENT_DENOMINATOR);
	current = (uint8_t)(temp & 0xFF);

	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQA_IDX);
	Chip_ADC_StartSequencer(LPC_ADC, ADC_SEQB_IDX);
}

void PININT0_IRQHandler(void)
{
	target_power_disable();
	target_set_over_current_flag();
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(TARGET_POWER_OC_INT));
}
