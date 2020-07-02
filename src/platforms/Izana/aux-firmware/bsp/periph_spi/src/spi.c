/*
 * @brief SPI example
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

#define RW_TEST             1
#define LOOPBACK_TEST       1
#define BUFFER_SIZE         10
#define SPI_MODE_TEST       (SPI_MODE_MASTER)
// #define SPI_MODE_TEST		(SPI_MODE_SLAVE)
#define POLLING_MODE        1
#if POLLING_MODE
#define INTERRUPT_MODE      0
#else
#define INTERRUPT_MODE      1
#endif

#define LPC_SPI           LPC_SPI1
#define SPI_IRQ           SPI1_IRQn
#define SPIIRQHANDLER     SPI1_IRQHandler

/* Tx buffer */
static uint16_t TxBuf[BUFFER_SIZE];

/* Rx buffer */
static uint16_t RxBuf[BUFFER_SIZE];

// static SPI_CONFIG_T ConfigStruct;
static SPI_DELAY_CONFIG_T DelayConfigStruct;

static SPI_DATA_SETUP_T XfSetup;
static volatile uint8_t  isXferCompleted = 0;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initializes pin muxing for SPI1 interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_SPI_PinMux(void)
{
#if (defined(BOARD_NXP_LPCXPRESSO_812) || defined(BOARD_LPC812MAX))
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	/*
	 * Initialize SSP0 pins connect
	 * SCK1: PINASSIGN4[31:24]: Select P0.12
	 * MOSI1: PINASSIGN5[7:0]: Select P0.14
	 * MISO1: PINASSIGN5[15:8] : Select P0.6
	 * SSEL1: PINASSIGN5[23:16]: Select P0.13
	 */
	Chip_SWM_DisableFixedPin(SWM_FIXED_VDDCMP);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, 12);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, 14);
#if defined(BOARD_LPC812MAX)
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 15);
#else
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 6);
#endif
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL_IO, 13);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#elif defined(BOARD_NXP_LPCXPRESSO_824)
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL0_IO, 15);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, 24);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 25);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, 26);
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#else
	/* Configure your own SPI pin muxing here if needed */
#warning "No SPI pin muxing defined"
#endif
}

/* Initialize buffer */
static void bufferInit(void)
{
	uint16_t i;
	uint16_t ch = 0;

	for (i = 0; i < BUFFER_SIZE; i++) {
		TxBuf[i] = ch++;
		RxBuf[i] = 0xAA;
	}
}

#if (RW_TEST || LOOPBACK_TEST)
/* Verify buffer after transfer */
static uint8_t bufferVerify(void)
{
	uint16_t i;
	uint16_t *src_addr = (uint16_t *) &TxBuf[0];
	uint16_t *dest_addr = (uint16_t *) &RxBuf[0];

	for ( i = 0; i < BUFFER_SIZE; i++ ) {

		if (*src_addr != *dest_addr) {
			return 1;
		}
		src_addr++;
		dest_addr++;
	}
	return 0;
}

#endif

/*****************************************************************************
 * Public functions
 ****************************************************************************/

#if INTERRUPT_MODE
/**
 * @brief	SPI interrupt handler sub-routine
 * @return	Nothing
 */
void SPIIRQHANDLER(void)
{
	Chip_SPI_Int_Cmd(LPC_SPI, SPI_INTENCLR_TXDYEN | SPI_INTENCLR_RXDYEN
					 | SPI_INTENCLR_RXOVEN | SPI_INTENCLR_TXUREN, DISABLE);						/* Disable all interrupt */
	if (((XfSetup.pRx) && (XfSetup.RxCnt < XfSetup.Length))
		|| ((XfSetup.pTx) && (XfSetup.TxCnt < XfSetup.Length))) {
		Chip_SPI_Int_RWFrames(LPC_SPI, &XfSetup);
		Chip_SPI_Int_Cmd(LPC_SPI, SPI_INTENSET_TXDYEN | SPI_INTENSET_RXDYEN
						 | SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN, ENABLE);
	}
	else {
		isXferCompleted = 1;
	}
}

#endif /*INTERRUPT_MODE*/

/**
 * @brief	Main routine for SPI example
 * @return	Nothing
 */
int main(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* SPI initialization */
	Init_SPI_PinMux();

	/*
	   ConfigStruct.Mode = SPI_MODE_TEST;
	   ConfigStruct.ClkDiv = Chip_SPI_CalClkRateDivider(LPC_SPI, 100000);
	   ConfigStruct.ClockMode = SPI_CLOCK_CPHA0_CPOL0;
	   ConfigStruct.DataOrder = SPI_DATA_MSB_FIRST;
	   ConfigStruct.SSELPol = SPI_SSEL_ACTIVE_LO;
	 */
	Chip_SPI_Init(LPC_SPI);
	Chip_SPI_ConfigureSPI(LPC_SPI, SPI_MODE_TEST |	/* Enable master/Slave mode */
						  SPI_CLOCK_CPHA0_CPOL0 |	/* Set Clock polarity to 0 */
						  SPI_CFG_MSB_FIRST_EN |/* Enable MSB first option */
						  SPI_CFG_SPOL_LO);	/* Chipselect is active low */

	DelayConfigStruct.FrameDelay = 0;
	DelayConfigStruct.PostDelay = 0;
	DelayConfigStruct.PreDelay = 0;
	DelayConfigStruct.TransferDelay = 0;
	Chip_SPI_DelayConfig(LPC_SPI, &DelayConfigStruct);

#if INTERRUPT_MODE
	/* Setting SPI interrupt */
	NVIC_EnableIRQ(SPI_IRQ);
#endif
	Chip_SPI_Enable(LPC_SPI);

#if LOOPBACK_TEST
	Chip_SPI_EnableLoopBack(LPC_SPI);
#endif
	bufferInit();
	XfSetup.Length = BUFFER_SIZE;
	XfSetup.pTx = TxBuf;
	XfSetup.RxCnt = XfSetup.TxCnt = 0;
	XfSetup.DataSize = 8;

#if (RW_TEST || LOOPBACK_TEST)
	XfSetup.pRx = RxBuf;
#else
	XfSetup.pRx = NULL;
#endif

#if POLLING_MODE
#if (RW_TEST || LOOPBACK_TEST)
	Chip_SPI_RWFrames_Blocking(LPC_SPI, &XfSetup);
#else
	Chip_SPI_WriteFrames_Blocking(LPC_SPI, &XfSetup);
#endif

#elif INTERRUPT_MODE
	Chip_SPI_Int_RWFrames(LPC_SPI, &XfSetup);
	Chip_SPI_Int_Cmd(LPC_SPI, SPI_INTENSET_TXDYEN | SPI_INTENSET_RXDYEN
					 | SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN, ENABLE);
	while (!isXferCompleted) {}
	while (!(Chip_SPI_GetStatus(LPC_SPI) & SPI_STAT_SSD)) {}/* Make sure the last frame sent completely */
#endif /*INTERRUPT_MODE*/

#if (RW_TEST || LOOPBACK_TEST)
	if (bufferVerify()) {
		Board_LED_Set(1, true);
	}
	else
#endif  /*(RW_TEST || LOOPBACK_TEST)*/
	{
		Board_LED_Set(0, true);
	}

#if LOOPBACK_TEST
	Chip_SPI_DisableLoopBack(LPC_SPI);
#endif
	/* DeInitialize SPI peripheral */
	Chip_SPI_DeInit(LPC_SPI);

	while (1) {}
}
