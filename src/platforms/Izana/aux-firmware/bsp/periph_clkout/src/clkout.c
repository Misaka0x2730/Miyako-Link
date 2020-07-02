/*
 * @brief CLKOUT example
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
// #include <stdio.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static uint32_t tick;

#define EVENT_TICK  0x01
static uint32_t event;

/* Board specific CLKOUT pins */
#if defined(BOARD_NXP_LPCXPRESSO_812)
#define CLKOUT_PIN 1
#else
#define CLKOUT_PIN 19
#endif

/* The Xpresso 824 does not have crystal connected as shipped. To enable move
 * jumpers SJ3 and SJ4 (check schematic for other 8xx boards) and define
 * SYS_OSC_CONNECTED 1
 * NOTE: Choosing a clock source that does not have an active clock will
 *       prevent the clockout selector from completing the switch until the
 *       part is reset. */
#define SYS_OSC_CONNECTED 0

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
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	/* bump the tick counter */
	tick++;

	/* each second toggle the LED */
	if ((tick % 100) == 0) {
		Board_LED_Toggle(0);
	}

	/* Every 30 seconds, notify the main app */
	if ((tick % 3000) == 0) {
		event |= EVENT_TICK;
	}
}

/**
 * @brief	main routine for CLKOUT example
 * @return	Function should not exit.
 */
int main(void)
{
	uint32_t clkIndex = 0;
	uint32_t maxIndex = 0;

	CHIP_SYSCTL_CLKOUTSRC_T clockSrc[] = {
		SYSCTL_CLKOUTSRC_MAINSYSCLK,
		SYSCTL_CLKOUTSRC_IRC,
		SYSCTL_CLKOUTSRC_WDTOSC,
		#if SYS_OSC_CONNECTED
		SYSCTL_CLKOUTSRC_SYSOSC,
		#endif
	};

	SystemCoreClockUpdate();
	Board_Init();

	Board_LED_Set(0, false);

	/* Enable and setup SysTick Timer at a 100Hz rate */
	SysTick_Config(SystemCoreClock / 100);

	/* Freq = 1.06Mhz, divided by 2. WDT_OSC should be ~530kHz */
	Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 2);

	/* Enable the power to WDT and clock to the WDT */
	Chip_SYSCTL_PowerUp(SYSCTL_SLPWAKE_WDTOSC_PD);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_WWDT);

	/* Configure PIN0.0 with pull-up */
	Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO1, PIN_MODE_PULLUP);

	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Connect the SWM_CLKOUT_O signal to port the pin */
	Chip_SWM_MovablePinAssign(SWM_CLKOUT_O, CLKOUT_PIN);

	 #if SYS_OSC_CONNECTED
	/* Enable power to the system osc */
	Chip_SYSCTL_PowerUp(SYSCTL_SLPWAKE_SYSOSC_PD);

	/* Set the P0.8 and P0.9 pin modes to no pull-up or pull-down */
	Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO8, PIN_MODE_INACTIVE);
	Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIO9, PIN_MODE_INACTIVE);

	/* Enable SYSOSC function on the pins */
	Chip_SWM_FixedPinEnable(SWM_FIXED_XTALIN, true);
	Chip_SWM_FixedPinEnable(SWM_FIXED_XTALOUT, true);
	#endif

	/* Disable the clock to the Switch Matrix */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Initialize clock source variables*/
	clkIndex = 0;
	maxIndex = (sizeof(clockSrc) / sizeof(clockSrc[0])) - 1;
	Chip_Clock_SetCLKOUTSource(clockSrc[clkIndex], 1);

	/* Cycle through all clock sources for the CLKOUT pin */
	while (1) {
		if (event & EVENT_TICK) {
			event &= ~EVENT_TICK;
			clkIndex = (clkIndex == maxIndex) ? 0 : clkIndex + 1;
			Chip_Clock_SetCLKOUTSource(clockSrc[clkIndex], 1);
		}
		__WFI();
	}
}
