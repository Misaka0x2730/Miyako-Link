/*
 * @brief Self Wake-up Timer (WKT) example
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

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void delay(uint32_t delay)
{
	volatile uint32_t i;
	for (i=0; i<delay; i++) {};
}

static void WakeupTest(WKT_CLKSRC_T clkSrc, uint32_t timeoutInSecs, CHIP_PMU_MCUPOWER_T powerTest)
{
	/* 10KHz clock source */
	Chip_WKT_SetClockSource(LPC_WKT, clkSrc);

	/* Setup for wakeup in timeout */
	Chip_WKT_LoadCount(LPC_WKT, Chip_WKT_GetClockRate(LPC_WKT) * timeoutInSecs);

	/* We can optionally call Chip_SYSCTL_SetDeepSleepPD() to power down the
	   BOD and WDT if we aren't using them in deep sleep modes. */
	Chip_SYSCTL_SetDeepSleepPD(SYSCTL_DEEPSLP_BOD_PD | SYSCTL_DEEPSLP_WDTOSC_PD);

	/* We should call Chip_SYSCTL_SetWakeup() to setup any peripherals we want
	   to power back up on wakeup. For this example, we'll power back up the IRC,
	   FLASH, the system oscillator, and the PLL */
	Chip_SYSCTL_SetWakeup(~(SYSCTL_SLPWAKE_IRCOUT_PD | SYSCTL_SLPWAKE_IRC_PD |
							SYSCTL_SLPWAKE_FLASH_PD | SYSCTL_SLPWAKE_SYSOSC_PD | SYSCTL_SLPWAKE_SYSPLL_PD));

	/* Tell PMU to go to sleep */
	Chip_PMU_Sleep(LPC_PMU, powerTest);

	/* Power anything back up here that isn't powered up on wakeup. The example
	   code below powers back up the BOD and WDT oscillator, which weren't setup to
	   pwower up in the Chip_SYSCTL_SetWakeup() function. */
	Chip_SYSCTL_SetDeepSleepPD(0);

	/* Will return here after wakeup and WKT IRQ, LED should be on */
	Chip_WKT_Stop(LPC_WKT);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from Wake-up timer
 * @return	Nothing
 */
void WKT_IRQHandler(void)
{
	/* Clear WKT interrupt request */
	Chip_WKT_ClearIntStatus(LPC_WKT);

	/* LED will toggle state on wakeup event */
	Board_LED_Toggle(0);
}

/**
 * @brief	Main program body
 * @return	Does not return
 */
int main(void)
{
	uint32_t regVal;

	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Alarm/wake timer as chip wakeup source */
	Chip_SYSCTL_EnablePeriphWakeup(SYSCTL_WAKEUP_WKTINT);

	/* Enable and reset WKT clock */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_WKT);
	Chip_SYSCTL_PeriphReset(RESET_WKT);

	/* Disable wakeup pad */
	Chip_PMU_ClearPowerDownControl(LPC_PMU, PMU_DPDCTRL_WAKEPAD | PMU_DPDCTRL_LPOSCDPDEN);

	/* Disable wakeup hysteresis by setting the bit (set to disable),
	   enable 10KHz oscillator for all power down modes including deep
	   power-down */
	Chip_PMU_SetPowerDownControl(LPC_PMU, PMU_DPDCTRL_WAKEUPPHYS | PMU_DPDCTRL_LPOSCEN |
								 PMU_DPDCTRL_LPOSCDPDEN);

	/* Enable WKT interrupt */
	NVIC_EnableIRQ(WKT_IRQn);

	/*
	 *	Note that deep power down causes a reset when it wakes up.
	 *	If the CPU was in deep power-down before the reset,
	 *	then PCON, DPDFLAG will be set.
	 *
	 *	This code clears DPDFLAG (by writing a one to it)
	 *	then sets the RED LED for about 500ms.
	 */
	if (LPC_PMU->PCON & PMU_PCON_DPDFLAG) {
		regVal = LPC_PMU->PCON;
		regVal |= PMU_PCON_DPDFLAG;
		LPC_PMU->PCON = regVal;
		Board_LED_Set(0, true);
		delay(0x100000);
	}

	/* Loop various tests */
	while (1) {
		/* You'll probably lose the debugger connection in the following
		   statements as the MCU goes into low power mode. */

		/* Wakeup test with 10KHz clock, 1s wakeup, and PMU sleep state */
		WakeupTest(WKT_CLKSRC_10KHZ, 1, PMU_MCU_SLEEP);

		/* Wakeup test with 10KHz clock, 1s wakeup, and PMU deep sleep state */
		WakeupTest(WKT_CLKSRC_10KHZ, 1, PMU_MCU_DEEP_SLEEP);

		/* Wakeup test with 10KHz clock, 1s wakeup, and PMU MCU power down state */
		WakeupTest(WKT_CLKSRC_10KHZ, 1, PMU_MCU_POWER_DOWN);

		/* Wakeup test with 10KHz clock, 1s wakeup, and PMU MCU deep power down state */
		WakeupTest(WKT_CLKSRC_10KHZ, 1, PMU_MCU_DEEP_PWRDOWN);
	}
}
