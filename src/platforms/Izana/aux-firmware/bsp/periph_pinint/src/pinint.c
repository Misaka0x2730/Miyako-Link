/*
 * @brief Pin Interrupt example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
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

/* Board specific definitions */
#if defined(BOARD_NXP_LPCXPRESSO_812)
/* Use same pin connected to potentiometer (PIO0) */
#define GPIO_PININT         0

#elif defined(BOARD_LPC812MAX)
#define GPIO_PININT         5

#elif defined(BOARD_NXP_LPCXPRESSO_824)
#define GPIO_PININT         4

#else
#warning "No GPIO assigned for this example"
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from PININT7
 * @return	Nothing
 */
void PININT7_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH7);
	Board_LED_Toggle(0);
}

/**
 * @brief	Main program body
 * @return	Does not return
 */
int main(void) {

	/* Generic Initialization */
	SystemCoreClockUpdate();

	/* Board_Init calls Chip_GPIO_Init, hence no need to
	   call Chip_GPIO_Init again */
	Board_Init();
	Board_LED_Set(0, false);

	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

#if defined(BOARD_NXP_LPCXPRESSO_812)
	/* Make sure fixed pin function is disabled and assign something safe to it */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I1);
	Chip_SWM_MovablePinAssign(SWM_CTIN_0_I, GPIO_PININT);
#elif defined(BOARD_NXP_LPCXPRESSO_824)
	/* Make sure fixed pin function is disabled and assign something safe to it */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ADC11);
	Chip_SWM_MovablePinAssign(SWM_SCT_IN1_I, GPIO_PININT);
#elif defined(BOARD_LPC812MAX)
	Chip_SWM_DisableFixedPin(SWM_FIXED_RST);
#endif

	/* Configure interrupt channel 7 for the GPIO pin in SysCon block */
	Chip_SYSCTL_SetPinInterrupt(7, GPIO_PININT);

	/* Configure channel 7 as wake up interrupt in SysCon block */
	Chip_SYSCTL_EnablePINTWakeup(7);

	/* Configure GPIO pin as input pin */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, 0, GPIO_PININT);

	/* Enable all clocks, even those turned off at wake power up */
	Chip_SYSCTL_SetWakeup(~(SYSCTL_SLPWAKE_IRCOUT_PD | SYSCTL_SLPWAKE_IRC_PD |
							SYSCTL_SLPWAKE_FLASH_PD | SYSCTL_SLPWAKE_SYSOSC_PD | SYSCTL_SLPWAKE_SYSOSC_PD |
							SYSCTL_SLPWAKE_SYSPLL_PD));

	/* Configure channel 7 interrupt as edge sensitive and falling edge interrupt */
	Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH7);
	Chip_PININT_EnableIntLow(LPC_PININT, PININTCH7);

	/* Enable interrupt in the NVIC */
	NVIC_EnableIRQ(PININT7_IRQn);

	/* Go to sleep mode - LED will toggle on each wakeup event */
	while (1) {
		Chip_PMU_SleepState(LPC_PMU);
	}
}
