/*
 * @brief periph PLL example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2015
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

#include <stdlib.h>

#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
 typedef struct {
	uint16_t	freq_main;			//	main clock frequency in MHz
	uint16_t	freq_sys;			//	system (CPU) clock frequency in MHz
	uint16_t	freq_fcco;			//	FCCO clock frequency in MHz
	uint16_t	msel;				//	MSEL (pre-decremented)
	uint16_t	psel;				//	PSEL (pre-decremented)
	uint16_t	divider;			//	SYSAHBCLKDIV
} LPC_8XX_PLL_T;
 
/*
 *	This table contains all useful PLL configurations
 *	for "integer" MHZ (e.g. 1MHz, 2MHz, etc.) frequencies.
 *
 *	This table has two inputs:
 *	- freq_main: This is the main frequency.
 *	- freq_sys: This is the system (CPU) frequency.
 *	These are used to select which table entry to use.
 *
 *	There are many ways to get some frequencies. For example,
 *	there are eight ways to make the CPU run at 12MHZ. If the peripheral bus
 *	needs to run very fast, it's possible to set the main clock
 *	up to 96MHz. If low power is a requirement, it's possible to set the main
 *	clock to 12MHz.
 *
 *	All the rest of the table entries are outputs.
 *	- freq_fcco is simply an FYI value. It is not used for programming.
 *	- MSEL / PSEL / divider are used to program the PLL.
 */
static const LPC_8XX_PLL_T test_tab[] = {
	{	  12,   12,  192,    0,    3,    1	},		// 12.0000MHz
	{	  12,    6,  192,    0,    3,    2	},		//  6.0000MHz
	{	  12,    4,  192,    0,    3,    3	},		//  4.0000MHz
	{	  12,    3,  192,    0,    3,    4	},		//  3.0000MHz
	{	  12,    2,  192,    0,    3,    6	},		//  2.0000MHz
	{	  12,    1,  192,    0,    3,   12	},		//  1.0000MHz
	{	  24,   24,  192,    1,    2,    1	},		// 24.0000MHz
	{	  24,   12,  192,    1,    2,    2	},		// 12.0000MHz
	{	  24,    8,  192,    1,    2,    3	},		//  8.0000MHz
	{	  24,    6,  192,    1,    2,    4	},		//  6.0000MHz
	{	  24,    4,  192,    1,    2,    6	},		//  4.0000MHz
	{	  24,    3,  192,    1,    2,    8	},		//  3.0000MHz
	{	  24,    2,  192,    1,    2,   12	},		//  2.0000MHz
	{	  24,    1,  192,    1,    2,   24	},		//  1.0000MHz
	{	  36,   18,  288,    2,    2,    2	},		// 18.0000MHz
	{	  36,   12,  288,    2,    2,    3	},		// 12.0000MHz
	{	  36,    9,  288,    2,    2,    4	},		//  9.0000MHz
	{	  36,    6,  288,    2,    2,    6	},		//  6.0000MHz
	{	  36,    4,  288,    2,    2,    9	},		//  4.0000MHz
	{	  36,    3,  288,    2,    2,   12	},		//  3.0000MHz
	{	  36,    2,  288,    2,    2,   18	},		//  2.0000MHz
	{	  36,    1,  288,    2,    2,   36	},		//  1.0000MHz
	{	  48,   24,  192,    3,    1,    2	},		// 24.0000MHz
	{	  48,   16,  192,    3,    1,    3	},		// 16.0000MHz
	{	  48,   12,  192,    3,    1,    4	},		// 12.0000MHz
	{	  48,    8,  192,    3,    1,    6	},		//  8.0000MHz
	{	  48,    6,  192,    3,    1,    8	},		//  6.0000MHz
	{	  48,    4,  192,    3,    1,   12	},		//  4.0000MHz
	{	  48,    3,  192,    3,    1,   16	},		//  3.0000MHz
	{	  48,    2,  192,    3,    1,   24	},		//  2.0000MHz
	{	  48,    1,  192,    3,    1,   48	},		//  1.0000MHz
	{	  60,   30,  240,    4,    1,    2	},		// 30.0000MHz
	{	  60,   20,  240,    4,    1,    3	},		// 20.0000MHz
	{	  60,   15,  240,    4,    1,    4	},		// 15.0000MHz
	{	  60,   12,  240,    4,    1,    5	},		// 12.0000MHz
	{	  60,   10,  240,    4,    1,    6	},		// 10.0000MHz
	{	  60,    6,  240,    4,    1,   10	},		//  6.0000MHz
	{	  60,    5,  240,    4,    1,   12	},		//  5.0000MHz
	{	  60,    4,  240,    4,    1,   15	},		//  4.0000MHz
	{	  60,    3,  240,    4,    1,   20	},		//  3.0000MHz
	{	  60,    2,  240,    4,    1,   30	},		//  2.0000MHz
	{	  60,    1,  240,    4,    1,   60	},		//  1.0000MHz
	{	  72,   24,  288,    5,    1,    3	},		// 24.0000MHz
	{	  72,   18,  288,    5,    1,    4	},		// 18.0000MHz
	{	  72,   12,  288,    5,    1,    6	},		// 12.0000MHz
	{	  72,    9,  288,    5,    1,    8	},		//  9.0000MHz
	{	  72,    8,  288,    5,    1,    9	},		//  8.0000MHz
	{	  72,    6,  288,    5,    1,   12	},		//  6.0000MHz
	{	  72,    4,  288,    5,    1,   18	},		//  4.0000MHz
	{	  72,    3,  288,    5,    1,   24	},		//  3.0000MHz
	{	  72,    2,  288,    5,    1,   36	},		//  2.0000MHz
	{	  72,    1,  288,    5,    1,   72	},		//  1.0000MHz
	{	  84,   28,  168,    6,    0,    3	},		// 28.0000MHz
	{	  84,   21,  168,    6,    0,    4	},		// 21.0000MHz
	{	  84,   14,  168,    6,    0,    6	},		// 14.0000MHz
	{	  84,   12,  168,    6,    0,    7	},		// 12.0000MHz
	{	  84,    7,  168,    6,    0,   12	},		//  7.0000MHz
	{	  84,    6,  168,    6,    0,   14	},		//  6.0000MHz
	{	  84,    4,  168,    6,    0,   21	},		//  4.0000MHz
	{	  84,    3,  168,    6,    0,   28	},		//  3.0000MHz
	{	  84,    2,  168,    6,    0,   42	},		//  2.0000MHz
	{	  84,    1,  168,    6,    0,   84	},		//  1.0000MHz
	{	  96,   24,  192,    7,    0,    4	},		// 24.0000MHz
	{	  96,   16,  192,    7,    0,    6	},		// 16.0000MHz
	{	  96,   12,  192,    7,    0,    8	},		// 12.0000MHz
	{	  96,    8,  192,    7,    0,   12	},		//  8.0000MHz
	{	  96,    6,  192,    7,    0,   16	},		//  6.0000MHz
	{	  96,    4,  192,    7,    0,   24	},		//  4.0000MHz
	{	  96,    3,  192,    7,    0,   32	},		//  3.0000MHz
	{	  96,    2,  192,    7,    0,   48	},		//  2.0000MHz
	{	  96,    1,  192,    7,    0,   96	},		//  1.0000MHz
};
static const uint16_t test_tab_ct = sizeof(test_tab) / sizeof(LPC_8XX_PLL_T);
static uint32_t tick;

#define EVENT_TICK  0x01
static uint32_t event;

#if defined(BOARD_NXP_LPCXPRESSO_824)
#define CLKOUT_PIN		24
#define IOCON_PIN		IOCON_PIO12	
#else
#define CLKOUT_PIN		12
#define IOCON_PIN		IOCON_PIO24
#endif
#define LED_RED			0
#define LED_GREEN		1
#define LED_BLUE		2

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static void LED_off(void)
{
	Board_LED_Set(LED_RED,		false);											// All LEDs off
	Board_LED_Set(LED_GREEN,	false);
	Board_LED_Set(LED_BLUE,		false);
}
static void LED_red(void)
{
	Board_LED_Set(LED_RED,		true);											// Red only
	Board_LED_Set(LED_GREEN,	false);
	Board_LED_Set(LED_BLUE,		false);
}
static void LED_green(void)
{
	Board_LED_Set(LED_RED,		false);
	Board_LED_Set(LED_GREEN,	true);											// Green only
	Board_LED_Set(LED_BLUE,		false);
}
static void LED_blue(void)
{
	Board_LED_Set(LED_RED,		false);
	Board_LED_Set(LED_GREEN,	false);
	Board_LED_Set(LED_BLUE,		true);											// Blue only
}

typedef enum {STATE_INIT, STATE_SYSTEM_CLK, STATE_MAIN_CLLK, STATE_STEP } SEQ_STATE_T;
static SEQ_STATE_T	s_state = STATE_INIT;
static uint16_t 	s_idx = 0;
static char			out_str[32];
static void test_sequencer(void)
{
	uint32_t main_clk = test_tab[s_idx].freq_main*1000000;
	uint32_t sys_clk = test_tab[s_idx].freq_sys*1000000;
	uint32_t div = test_tab[s_idx].divider;

	switch (s_state) {															// state machine processing
	case STATE_INIT:															// state machine initialization
		LED_off();																// LED off
		Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, 0);				// disable clock out
		s_idx = 0;																// init to the start of the table
		s_state = STATE_SYSTEM_CLK;												// start with main clock
		break;

	case STATE_SYSTEM_CLK:														// run the main clock
		LED_red();																// show red LED
		Chip_IRC_SetFreq(main_clk, sys_clk);									// set frequency
		Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, 1);				// set the output divider
		SystemCoreClockUpdate();												// update the clock
		SysTick_Config(SystemCoreClock / 100);									// update systick
		Board_Debug_Init();														// init UART
		
		Board_UARTPutSTR("Table index:");
		Board_itoa(s_idx, out_str, 10);
		Board_UARTPutSTR(out_str);

		Board_UARTPutSTR(" main clock:");
		Board_itoa((main_clk/1000000), out_str, 10);
		Board_UARTPutSTR(out_str);

		Board_UARTPutSTR(" system clock:");
		Board_itoa((sys_clk/1000000), out_str, 10);
		Board_UARTPutSTR(out_str);
		Board_UARTPutChar(0x0a);
		
		s_state = STATE_MAIN_CLLK;												// step to system clock
		break;

	case STATE_MAIN_CLLK:														// run the system clock
		LED_green();															// show green LED
		Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, div);
		s_state = STATE_STEP;													// step to the next clock
		break;

	case STATE_STEP:															// step to the next configuration
		LED_blue();																// show blue LED
		Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_MAINSYSCLK, 0);				// disable clock out
		s_idx += 1;																// bump the index
		if (s_idx == test_tab_ct) s_idx = 0;									// test / loop the index
		s_state = STATE_SYSTEM_CLK;												// step to the next clock
		break;
	}
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
	/* bump the tick counter */
	tick++;

	/* Every 30 seconds, notify the main app */
	if ((tick % 300) == 0) {
		event |= EVENT_TICK;
	}
}

/**
 * @brief	main routine for CLKOUT example
 * @return	Function should not exit.
 */
int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	Board_UARTPutSTR("PLL/IRC Frequency table demo:\n");
	Board_UARTPutSTR("build date: " __DATE__ " build time: " __TIME__ "\n");

	/* Enable and setup SysTick Timer at a 100Hz rate */
	SysTick_Config(SystemCoreClock / 100);

	/* Freq = 1.06Mhz, divided by 2. WDT_OSC should be ~530kHz */
	Chip_Clock_SetWDTOSC(WDTLFO_OSC_1_05, 2);

	/* Enable the power to WDT and clock to the WDT */
	Chip_SYSCTL_PowerUp(SYSCTL_SLPWAKE_WDTOSC_PD);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_WWDT);

	Chip_IOCON_PinSetMode(LPC_IOCON, IOCON_PIN, PIN_MODE_PULLUP);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_MovablePinAssign(SWM_CLKOUT_O, CLKOUT_PIN);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Cycle through all clock sources for the CLKOUT pin */
	while (1) {
		if (event & EVENT_TICK) {
			event &= ~EVENT_TICK;
			test_sequencer();
		}
		__WFI();
	}
}
