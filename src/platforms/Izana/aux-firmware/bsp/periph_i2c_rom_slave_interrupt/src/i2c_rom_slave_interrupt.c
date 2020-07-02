/*
 * @brief I2C bus slave example using the ROM API
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
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

/* I2C master handle and memory for ROM API */
static I2C_HANDLE_T *i2cHandleSlave;

/* Use a buffer size larger than the expected return value of
   i2c_get_mem_size() for the static I2C handle type */
static uint32_t i2cSlaveHandleMEM[0x20];

/** I2C addresses - in slave mode, only 7-bit addressing is supported */
#define I2C_ADDR_8BIT       (0xc0)
#define I2C_BUFF_SZ         16

/* Receive and transmit buffers */
static uint8_t rxBuff[I2C_BUFF_SZ];
static uint8_t txBuff[I2C_BUFF_SZ];

/* Global I2C ROM API parameter and results structures */
static I2C_PARAM_T param;
static I2C_RESULT_T result;

static void I2C_ParamCfg(void);

static void I2C_ResultClr(void);

static void I2C_SetupXfer(void);

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Converts given value @a val into a hexadecimal string
 * and stores the result to @a dest with leading zeros
 * RETURN: Number of hexdigits excluding leading zeros
 */
static int Hex2Str(char *dest, uint32_t val)
{
	int i, ret = 0;
	for (i = 0; i < sizeof(val) * 2; i ++) {
		int idx = val & 0xF;
		dest[7 - i] = "0123456789ABCDEF"[idx];
		val >>= 4;
		if (idx)
			ret = i;
	}
	return ret + 1;
}

/* Prints a hexadecimal value with given string in front */
/* Using printf might cause text section overflow */
static void Print_Val(const char *str, uint32_t val)
{
	char buf[9];
	int ret;
	buf[8] = 0;
	DEBUGSTR(str);
	ret = Hex2Str(buf, val);
	DEBUGSTR(&buf[8 - ret]);
	DEBUGSTR("\r\n");
}

/* Prints an 8-bit value */
static void Print_Val8(uint8_t val)
{
	char buf[] = "XX ";
	buf[0] = "0123456789ABCDEF"[val >> 4];
	buf[1] = "0123456789ABCDEF"[val & 0x0F];
	DEBUGSTR(buf);
}

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

/* Turn on LED to indicate an error */
static void I2C_DieError(const char *msg, ErrorCode_t erno)
{
	DEBUGSTR("ERROR: ");
	DEBUGSTR(msg);
	Print_Val(": Error number: 0x", erno);
	while (1) {
		__WFI();
	}
}

/* Setup I2C handle and parameters */
static void I2C_SetupSlaveMode(void)
{
	ErrorCode_t error_code;

	/* Enable I2C clock and reset I2C peripheral */
	DEBUGSTR("I2C slave mode set up:\r\n");

	Chip_I2C_Init(LPC_I2C);

	/* Perform a sanity check on the storage allocation */
	DEBUGSTR(" - Testing memory size: ");
	if (LPC_I2CD_API->i2c_get_mem_size() > sizeof(i2cSlaveHandleMEM)) {
		I2C_DieError("Not enough memory", ERR_I2C_GENERAL_FAILURE);
	}
	DEBUGSTR("pass\r\n");

	/* Setup the I2C handle */
	DEBUGSTR(" - Configure slave handle: ");
	i2cHandleSlave = LPC_I2CD_API->i2c_setup(LPC_I2C_BASE, i2cSlaveHandleMEM);
	if (i2cHandleSlave == NULL) {
		I2C_DieError("i2c_setup", ERR_I2C_GENERAL_FAILURE);
	}
	DEBUGSTR("pass\r\n");

	/* Set a single 7-bit I2C address, only 7-bit addressing is supported */
	DEBUGSTR(" - Configure slave address: ");
	error_code = LPC_I2CD_API->i2c_set_slave_addr(i2cHandleSlave, I2C_ADDR_8BIT, 0);
	if (error_code != LPC_OK) {
		DEBUGSTR("Error setting I2C slave address\r\n");
		I2C_DieError("i2c_set_slave_addr", ERR_I2C_GENERAL_FAILURE);
	}
	DEBUGSTR("pass\r\n");

	DEBUGSTR(" - Parameter configuration\r\n");
	I2C_ParamCfg();

	DEBUGSTR(" - Result clear\r\n");
	I2C_ResultClr();
}

/* I2C interrupt receive callback, called on completion of I2C 'read'
   operation when in interrupt mode. Called in interrupt context. */
static void I2C_XferComplete(uint32_t err_code, uint32_t n)
{
	I2C_SetupXfer();
	DEBUGSTR("I2C transfer complete\r\n");
}

static void I2C_ParamCfg(void)
{
	param.func_pt = I2C_XferComplete;
	param.num_bytes_send = I2C_BUFF_SZ;
	param.buffer_ptr_send = txBuff;
	param.num_bytes_rec = I2C_BUFF_SZ;
	param.buffer_ptr_rec = rxBuff;
}

static void I2C_ResultClr(void)
{
	result.n_bytes_sent = result.n_bytes_recd = 0;
}

/* Slave transmit in interrupt mode */
static void I2C_SetupXfer(void)
{
	ErrorCode_t er;
	uint8_t data;

	uint8_t *buff_ptr = NULL;
	uint32_t i;

	if (result.n_bytes_recd > 0) {
		DEBUGSTR("RX data: ");
		for (buff_ptr = rxBuff, i = 0; i < result.n_bytes_recd; i++) {
			Print_Val8(*buff_ptr++);
		}
		DEBUGSTR("\r\n");
	}

	if (result.n_bytes_sent > 0) {
		DEBUGSTR("TX data: ");
		for (buff_ptr = txBuff, i = 0; i < result.n_bytes_sent; i++) {
			Print_Val8(*buff_ptr++);
		}
		DEBUGSTR("\r\n");
	}

	if (result.n_bytes_recd >= 2) {
		txBuff[0] = data = rxBuff[result.n_bytes_recd - 1];
		Board_LED_Set(0, data);
	}

	I2C_ResultClr();

	er = LPC_I2CD_API->i2c_slave_transmit_intr(i2cHandleSlave, &param, &result);
	if (er != LPC_OK) {
		I2C_DieError("i2c_slave_transmit_intr", er);
	}

	er = LPC_I2CD_API->i2c_slave_receive_intr(i2cHandleSlave, &param, &result);
	if (er != LPC_OK) {
		I2C_DieError("i2c_slave_transmit_intr", er);
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	I2C interrupt handler
 * @return	Nothing
 */
void I2C_IRQHandler(void)
{
	/* Call I2C ISR function in ROM with the I2C handle */
	DEBUGSTR("I2C interrupt\r\n");
	LPC_I2CD_API->i2c_isr_handler(i2cHandleSlave);
}

/**
 * @brief	Main routine for I2C example
 * @return	Function should not exit
 */
int main(void)
{
	uint32_t fw = 0;
	/* Generic Initialization */
	SystemCoreClockUpdate();
	Board_Init();

	DEBUGSTR("I2C ROM slave interrupt test\r\n");
	Print_Val("I2C slave address: 0x", I2C_ADDR_8BIT);
	fw = LPC_I2CD_API->i2c_get_firmware_version();
	Print_Val("I2C firmware version: 0x", fw);

	/* Set initial LED state to off */
	Board_LED_Set(0, true);

	/* Setup I2C at the board level (usually pin muxing) */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C clocking */
	I2C_SetupSlaveMode();

	/* Enable the interrupt for the I2C */
	NVIC_EnableIRQ(I2C_IRQn);

	/* Setup I2C receive slave mode - this will setup a
	   non-blocking I2C mode which will be handled via the I2C interrupt */
	I2C_SetupXfer();

	/* I2C slave handler loop - wait for requests from master and
	   receive or send data in response */
	while (1) {
		/* Sleep while waiting for I2C master requests */
		__WFI();

		/* All I2C slave processing is performed in the I2C IRQ
		   handler, so there is nothing to do here */
	}
}
