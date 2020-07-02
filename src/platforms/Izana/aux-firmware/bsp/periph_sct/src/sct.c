/*
 * @brief State Configurable Timer (SCT) example
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

#define TICKRATE_HZ (10)	/* 10 ticks per second */

static volatile uint32_t ticks;

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
	++ticks;
}

/**
 * @brief	Handle interrupt from State Configurable Timer
 * @return	Nothing
 */
void SCT_IRQHandler(void)
{
	if (ticks % 2) {
		Board_LED_Toggle(2);
	}
	else {
		Board_LED_Toggle(0);
	}

	/* Clear the SCT Event 0 Interrupt */
	Chip_SCT_ClearEventFlag(LPC_SCT, SCT_EVT_0);
}

/**
 * @brief	Application main program
 * @return	Nothing (This function will not return)
 */
int main(void)
{
	/* Generic Initialization */
 	SystemCoreClockUpdate();
	Board_Init();

	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	/* Custom Initialization */

	/* Initialize the SCT clock and reset the SCT */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SCT);
	Chip_SYSCTL_PeriphReset(RESET_SCT);

	/* Configure the SCT counter as a unified (32 bit) counter using the bus clock */
	Chip_SCT_Config(LPC_SCT, SCT_CONFIG_32BIT_COUNTER | SCT_CONFIG_CLKMODE_BUSCLK);

	/* The match/capture REGMODE defaults to match mode */
	/* No REGMODE changes are needed for this program   */

	/* Set the match count for match register 0 */
	Chip_SCT_SetMatchCount(LPC_SCT, SCT_MATCH_0, SystemCoreClock / TICKRATE_HZ);

	/* Set the match reload value for match reload register 0*/
	Chip_SCT_SetMatchReload(LPC_SCT, SCT_MATCH_0, SystemCoreClock / TICKRATE_HZ);

	/* Event 0 only happens on a match condition */
	LPC_SCT->EV[0].CTRL = (1 << 12);

	/* Event 0 only happens in state 0 */
	LPC_SCT->EV[0].STATE = 0x00000001;

	/* Event 0 is used as the counter limit */
	LPC_SCT->LIMIT_U = 0x00000001;

	/* Enable flag to request an interrupt for Event 0 */
	Chip_SCT_EnableEventInt(LPC_SCT, SCT_EVT_0);

	/* Enable the interrupt for the SCT */
	NVIC_EnableIRQ(SCT_IRQn);

	/* Start the SCT counter by clearing Halt_L in the SCT control register */
	Chip_SCT_ClearControl(LPC_SCT, SCT_CTRL_HALT_L);

	while (1) {
		__WFI();
	}
}
