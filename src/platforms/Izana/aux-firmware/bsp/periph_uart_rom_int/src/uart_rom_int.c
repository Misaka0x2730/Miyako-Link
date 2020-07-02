/*
 * @brief UART API in ROM (USART API ROM) interrupt example
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
#include <string.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* UART handle and memory for ROM API */
static UART_HANDLE_T *uartHandle;

/* Use a buffer size larger than the expected return value of
   uart_get_mem_size() for the static UART handle type */
static uint32_t uartHandleMEM[0x10];

/* Receive buffer */
#define RECV_BUFF_SIZE 32
static char recv_buf[RECV_BUFF_SIZE];
static int indexIn;

/* ASCII code for escape key */
#define ESCKEY			27
#define	UART_CLOCK_DIV	1

/* Flag used to indicate callback fired */
static volatile bool gotCallback;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* UART Pin mux function - note that SystemInit() may already setup your
   pin muxing at system startup */
static void Init_UART_PinMux(void)
{
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	Chip_Clock_SetUARTClockDiv(UART_CLOCK_DIV);

#if (defined(BOARD_NXP_LPCXPRESSO_812) || defined(BOARD_LPC812MAX) || defined(BOARD_NXP_LPCXPRESSO_824))
	/* Connect the U0_TXD_O and U0_RXD_I signals to port pins(P0.4, P0.0) */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I1);
	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 4);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 0);
#else
	/* Configure your own UART pin muxing here if needed */
#warning "No UART pin muxing defined"
#endif

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
}

/* Turn on LED to indicate an error */
static void errorUART(void)
{
	Board_LED_Set(0, true);
	while (1) {}
}

/* Setup UART handle and parameters */
static void setupUART()
{
	uint32_t frg_mult;

	/* 115.2KBPS, 8N1, ASYNC mode, no errors, clock filled in later */
	UART_CONFIG_T cfg = {
		0,				/* U_PCLK frequency in Hz */
		115200,			/* Baud Rate in Hz */
		1,				/* 8N1 */
		0,				/* Asynchronous Mode */
		NO_ERR_EN		/* Enable No Errors */
	};

	/* Perform a sanity check on the storage allocation */
	if (LPC_UARTD_API->uart_get_mem_size() > sizeof(uartHandleMEM)) {
		/* Example only: this should never happen and probably isn't needed for
		   most UART code. */
		errorUART();
	}

	/* Setup the UART handle */
	uartHandle = LPC_UARTD_API->uart_setup((uint32_t) LPC_USART0, (uint8_t *) &uartHandleMEM);
	if (uartHandle == NULL) {
		errorUART();
	}

	/* Need to tell UART ROM API function the current UART peripheral clock speed */
	cfg.sys_clk_in_hz = Chip_Clock_GetMainClockRate()/UART_CLOCK_DIV;

	/* Initialize the UART with the configuration parameters */
	frg_mult = LPC_UARTD_API->uart_init(uartHandle, &cfg);
	if (frg_mult) {
		Chip_SYSCTL_SetUSARTFRGDivider(0xFF);	/* value 0xFF should be always used */
		Chip_SYSCTL_SetUSARTFRGMultiplier(frg_mult);
	}
}

/* UART callback */
static void waitCallback(uint32_t err_code, uint32_t n)
{
	gotCallback = true;
}

/* Sleep until callback is called */
void sleepUntilCB(void)
{
	/* Wait until the transmit callback occurs. When it hits, the
	     transfer is complete. */
	while (gotCallback == false) {
		/* Sleep until the callback signals transmit completion */
		__WFI();
	}
}

/* Send a string on the UART terminated by a NULL character using
   polling mode. */
static void putLineUART(const char *send_data)
{
	UART_PARAM_T param;

	param.buffer = (uint8_t *) send_data;
	param.size = strlen(send_data);

	/* Interrupt mode, do not append CR/LF to sent data */
	param.transfer_mode = TX_MODE_SZERO;
	param.driver_mode = DRIVER_MODE_INTERRUPT;

	/* Setup the transmit callback, this will get called when the
	   transfer is complete */
	param.callback_func_pt = (UART_CALLBK_T) waitCallback;

	/* Transmit the data using interrupt mode, the function will
	   return */
	gotCallback = false;
	if (LPC_UARTD_API->uart_put_line(uartHandle, &param)) {
		errorUART();
	}

	/* Wait until the transmit callback occurs. When it hits, the
	   transfer is complete. */
	sleepUntilCB();
}

/* Receive a string on the UART terminated by a LF character using
   polling mode. */
static void getLineUART(char *receive_buffer, uint32_t length)
{
	UART_PARAM_T param;

	param.buffer = (uint8_t *) receive_buffer;
	param.size = length;

	/* Receive data up to the CR/LF character in polling mode. Will
	   truncate at length if too long.	*/
	param.transfer_mode = RX_MODE_CRLF_RECVD;
	param.driver_mode = DRIVER_MODE_INTERRUPT;

	/* Setup the receive callback, this will get called when the
	   transfer is complete */
	param.callback_func_pt = (UART_CALLBK_T) waitCallback;

	/* Receive the data */
	gotCallback = false;
	if (LPC_UARTD_API->uart_get_line(uartHandle, &param)) {
		errorUART();
	}

	/* Wait until the transmit callback occurs. When it hits, the
	   transfer is complete. */
	sleepUntilCB();
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle UART interrupt
 * @return	Nothing
 */
void UART0_IRQHandler(void)
{
	LPC_UARTD_API->uart_isr(uartHandle);
}

/**
 * @brief	Main Function
 * @return	Always returns 0
 */
int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	Init_UART_PinMux();
	Chip_UART_Init(LPC_USART0);
	Board_LED_Set(0, false);

	/* Allocate UART handle, setup UART parameters, and initialize UART
	   clocking */
	setupUART();

	/* Enable the IRQ for the UART */
	NVIC_EnableIRQ(UART0_IRQn);

	/* Transmit the welcome message and instructions using the
	   putline function with a callback */
	putLineUART("LPC8XX USART API ROM interrupt Example\r\n");
	putLineUART("Enter a string, press Enter (CR + LF) to echo it back:\r\n");

	/* Get a string for the UART and echo it back to the caller. Data is NOT
	   echoed back via the UART using this function. */
	getLineUART(recv_buf, sizeof(recv_buf));
	recv_buf[sizeof(recv_buf) - 1] = '\0';	/* Safety */
	if (strlen(recv_buf) == (sizeof(recv_buf) - 1)) {
		putLineUART("**String was truncated, input data longer than "
					"receive buffer***\r\n");
	}
	putLineUART(recv_buf);

	/* Transmit the message for byte/character part of the example */
	putLineUART("\r\nByte receive with echo using uart_get_line(): "
				"Send 16 bytes to fill the ring buffer\r\n");

	/* An (inefficient) ring buffer can be emulated in interrupt
	   mode using the uart_get_line() with a buffer size of 1. Read
	   16 bytes into the ring buffer and then exit.	*/
	indexIn = 0;
	while (indexIn < 16) {
		/* Get byte in next index in buffer */
		getLineUART(&recv_buf[indexIn], 1);

		/* Echo back using polling function */
		LPC_UARTD_API->uart_put_char(uartHandle, recv_buf[indexIn]);

		/* Next index */
		indexIn++;
	}

	/* Safe string termination */
	recv_buf[indexIn] = '\0';

	/* Transmit the message for byte/character part of the example */
	putLineUART("\r\nData in ring buffer: [");
	putLineUART(recv_buf);
	putLineUART("]\r\n");

	return 0;
}
