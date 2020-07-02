/*
 * @brief I2CM bus master example using interrupt mode
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

/* I2CM transfer record */
static I2CM_XFER_T  i2cmXferRec;
/* System clock is set to 24MHz, I2C clock is set to 600kHz */
#define I2C_CLK_DIVIDER         (40)
/* 100KHz I2C bit-rate */
#define I2C_BITRATE             (100000)

/* 7-bit I2C addresses of I/O expander */
/* Note: The ROM code requires the address to be between bits [6:0]
         bit 7 is ignored */
#define I2C_ADDR_7BIT           (0x60)

/* SysTick rate in Hz */
#define TICKRATE_HZ             (1000)

#define TASK_LOOP               while (true)
#define EVENT_LED_BUMP          0x01
#define EVENT_LED_TOGGLE        0x02

/* file local variables */
static volatile int intErrCode;
static volatile uint32_t ticks;
static uint8_t txData[16];
static uint8_t rxData[16];
static uint32_t event_flag;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes pin muxing for I2C interface */
static void Init_I2C_PinMux(void)
{
#if (defined(BOARD_NXP_LPCXPRESSO_812) || defined(BOARD_LPC812MAX) || defined(BOARD_NXP_LPCXPRESSO_824))
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
#if defined(BOARD_NXP_LPCXPRESSO_824)
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
#else
	/* Connect the I2C_SDA and I2C_SCL signals to port pins(P0.10, P0.11) */
	Chip_SWM_MovablePinAssign(SWM_I2C_SDA_IO, 10);
	Chip_SWM_MovablePinAssign(SWM_I2C_SCL_IO, 11);
#endif

	/* Enable Fast Mode Plus for I2C pins */
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO10, PIN_I2CMODE_FASTPLUS);
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO11, PIN_I2CMODE_FASTPLUS);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#else
	/* Configure your own I2C pin muxing here if needed */
#warning "No I2C pin muxing defined"
#endif
}

/* Setup I2C handle and parameters */
static void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral */
	Chip_I2C_Init(LPC_I2C);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(LPC_I2C, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(LPC_I2C, I2C_BITRATE);

	/* Enable Master Mode */
	Chip_I2CM_Enable(LPC_I2C);
}

/* Function to wait for I2CM transfer completion */
static void WaitForI2cXferComplete(I2CM_XFER_T *xferRecPtr)
{
	/* Test for still transferring data */
	while (xferRecPtr->status == I2CM_STATUS_BUSY) {
		/* Sleep until next interrupt */
		__WFI();
	}
}

/* Function to setup and execute I2C transfer request */
static void SetupXferRecAndExecute(uint8_t devAddr,
								   uint8_t *txBuffPtr,
								   uint16_t txSize,
								   uint8_t *rxBuffPtr,
								   uint16_t rxSize)
{
	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_Xfer(LPC_I2C, &i2cmXferRec);
	/* Enable Master Interrupts */
	Chip_I2C_EnableInt(LPC_I2C, I2C_INTENSET_MSTPENDING | I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR);
	/* Wait for transfer completion */
	WaitForI2cXferComplete(&i2cmXferRec);
	/* Clear all Interrupts */
	Chip_I2C_ClearInt(LPC_I2C, I2C_INTENSET_MSTPENDING | I2C_INTENSET_MSTRARBLOSS | I2C_INTENSET_MSTSTSTPERR);
}

/* Function to build the LED value for the PCA9532 LED driver & I/O expander */
static uint32_t build_led(uint32_t t1)
{
	uint32_t i, k, out_val = 0;

	for (i = 0; i < 16; i++) {
		k = ((t1 >> i) & 0x01) ? 0x01 : 0x00;
		out_val |= (k << (i * 2));
	}

	return out_val;
}

/* Function sends update to the I/O expander */
static void sendI2CMaster(uint16_t i2c_addr, uint32_t ledStateOut)
{
	int index = 0;

	txData[index++] = (uint8_t) 0x16;							/* I2C device regAddr */
	txData[index++] = (uint8_t) ((ledStateOut)     & 0xff);		/* I2C device regVal */
	txData[index++] = (uint8_t) ((ledStateOut >> 8)  & 0xff);		/* I2C device regVal */
	txData[index++] = (uint8_t) ((ledStateOut >> 16) & 0xff);		/* I2C device regVal */
	txData[index++] = (uint8_t) ((ledStateOut >> 24) & 0xff);		/* I2C device regVal */

	SetupXferRecAndExecute(i2c_addr, txData, 5, rxData, 0);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle I2C interrupt by calling I2CM interrupt transfer handler
 * @return	Nothing
 */
void I2C_IRQHandler(void)
{
	/* Call I2CM ISR function with the I2C device and transfer rec */
	Chip_I2CM_XferHandler(LPC_I2C, &i2cmXferRec);
}

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	ticks++;

	if ((ticks % 50) == 0) {
		event_flag |= EVENT_LED_BUMP;
	}
	if ((ticks % 125) == 0) {
		event_flag |= EVENT_LED_TOGGLE;
	}
}

/**
 * @brief	Main routine for I2C example
 * @return	Function should not exit
 */
int main(void)
{
	uint32_t led_ct = 0;

	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	/* Clear activity LED */
	Board_LED_Set(0, false);

	/* Setup I2C pin muxing */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C clocking */
	setupI2CMaster();

	/* Enable the interrupt for the I2C */
	NVIC_EnableIRQ(I2C_IRQn);

	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

	/* Enter the task loop */
	TASK_LOOP {
		__WFI();

		/* Bump the off-board LEDs by one */
		if (event_flag & EVENT_LED_BUMP) {
			event_flag &= ~EVENT_LED_BUMP;
			sendI2CMaster(I2C_ADDR_7BIT, build_led(led_ct++));
		}

		/* Toggle on-board RED LED to show activity. */
		if (event_flag & EVENT_LED_TOGGLE) {
			event_flag &= ~EVENT_LED_TOGGLE;
			Board_LED_Toggle(0);
		}
	}
}
