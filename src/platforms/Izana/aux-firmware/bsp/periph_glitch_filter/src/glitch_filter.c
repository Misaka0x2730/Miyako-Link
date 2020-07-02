/*
 * @brief Digital I/O glitch-filter example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* SCT configuration */
#define SCT_PWM             LPC_SCT
#define SCT_PWM_PIN_LED     0				/* COUT0 [index 2] Controls LED */
#define SCT_PWM_LED         2				/* Index of LED PWM */
#define SCT_PWM_RATE        100000			/* PWM frequency 100 kHz */

/* Systick timer tick rate */
#define TICKRATE_HZ         100				/* 10 ms Tick rate */

/* Events to process */
#define EVENT_TICK          0x01
#define EVENT_GLITCH        0x02
static uint32_t event_flag;

/* Board definitions */
#if (defined(BOARD_NXP_LPCXPRESSO_812) || defined(BOARD_LPC812MAX))
#define RED_LED_PIN         7
#define RED_LED_IOCON       IOCON_PIO7
#define SWM_PWM_OUT         SWM_CTOUT_0_O

#elif defined(BOARD_NXP_LPCXPRESSO_824)
#define RED_LED_PIN         12
#define RED_LED_IOCON       IOCON_PIO12
#define SWM_PWM_OUT         SWM_SCT_OUT0_O

#else
#warning "No LED pin defined"
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Setup board specific pin muxing */
static void app_setup_pin(void)
{
	/* Set the PWM output to the RED LED */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_MovablePinAssign(SWM_PWM_OUT, RED_LED_PIN);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	event_flag |= EVENT_TICK;
}

/**
 * @brief	Handle interrupt from PININT channel 0
 * @return	Nothing
 */
void PININT0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH0);
	event_flag |= EVENT_GLITCH;
}

/* Example entry point */
int main(void)
{
	int led_dp = 0, led_step = 1;

	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Setup Board specific output pin */
	app_setup_pin();

	Board_LED_Set(0, false);	/* shut off red LED */
	Board_LED_Set(1, false);	/* shut off green LED */

	/* Initialize glitch-filter and assign to RED LED */
	Chip_Clock_SetIOCONCLKDIV(IOCONCLKDIV0, 40);							/* set divider to 40 this yields a 600kHz sample clock */
	Chip_IOCON_PinSetClockDivisor(LPC_IOCON, RED_LED_IOCON, IOCONCLKDIV0);	/* assign IOCON clock divider 0 to RED LED pin */
	Chip_IOCON_PinSetSampleMode(LPC_IOCON, RED_LED_IOCON, PIN_SMODE_CYC1);	/* enable the 1 cycle glitch mode */

	/* Initialize pin interrupt */
	Chip_SYSCTL_SetPinInterrupt(0, RED_LED_PIN);							/* Set pin interrupt 0 to RED LED pin*/
	Chip_PININT_Init(LPC_PININT);											/* initialze pin interrupt module */
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH0);						/* set pin interrupt channel 0 to be edge sensitive */
	Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH0);						/* interrupt on rising edge */
	NVIC_EnableIRQ(PININT0_IRQn);											/* enable pin interrupt 0 */

	/* Initialize the SCT as PWM and set frequency */
	Chip_SCTPWM_Init(SCT_PWM);
	Chip_SCTPWM_SetRate(SCT_PWM, SCT_PWM_RATE);
	Chip_SCTPWM_SetOutPin(SCT_PWM, SCT_PWM_LED, SCT_PWM_PIN_LED);
	Chip_SCTPWM_SetDutyCycle(SCT_PWM, SCT_PWM_LED, 0);
	Chip_SCTPWM_Start(SCT_PWM);

	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	while (1) {
		/* on the tick event */
		if (event_flag & EVENT_TICK) {
			event_flag &= ~EVENT_TICK;

			led_dp += led_step;
			if (led_dp == 0) {
				led_step = 1;
			}
			if (led_dp == 200) {
				led_step = -1;
			}

			/* Increment or decrement duty-cycle 1% every 20ms */
			Chip_SCTPWM_SetDutyCycle(SCT_PWM, SCT_PWM_LED,
									 Chip_SCTPWM_PercentageToTicks(SCT_PWM, led_dp) / 2);
		}

		/* on the glitch event */
		if (event_flag & EVENT_GLITCH) {
			event_flag &= ~EVENT_GLITCH;
			Board_LED_Set(1, true);		/* light green LED */
			Board_LED_Set(1, false);	/* shut off green LED */
		}

		__WFI();
	}
}
