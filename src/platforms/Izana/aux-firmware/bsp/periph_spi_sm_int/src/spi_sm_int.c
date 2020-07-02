/*
 * @brief SPI master and slave example
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
#include "stopwatch.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* Stopwatch timers for master and slave transfer time */
static uint32_t masterTime, errors;

/* Flags for determing when master and slave transfers end */
static volatile bool mEnd, sEnd;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/* Local variables, made global to make easier to debug */

/* Buffer sizes for this example and total transfer size */
#define BUFFER_SIZE 16

/* Master transmit and receive buffers */
uint16_t masterRXBuffer16[BUFFER_SIZE], masterTXBuffer16[BUFFER_SIZE];

/* SPI master transfer descriptor */
SPIM_XFER_T spiMasterXfer;

/* Slave transmit and receive buffers */
uint16_t slaveRXBuffer16[BUFFER_SIZE], slaveTXBuffer16[BUFFER_SIZE];

/* SPI slave transfer descriptor */
SPIS_XFER_T spiSlaveXfer;

/* Board specific example setup */
#define SPIMASTERIRQHANDLER                 SPI0_IRQHandler
#define LPC_SPIMASTERPORT                   LPC_SPI0
#define LPC_SPIMASTERIRQNUM                 SPI0_IRQn
#define SPISLAVEIRQHANDLER                  SPI1_IRQHandler
#define LPC_SPISLAVEPORT                    LPC_SPI1
#define LPC_SPISLAVEIRQNUM                  SPI1_IRQn

/* The slave can handle data up to the point of overflow or underflow.
   Adjust this clock rate and the master's timing delays to get a working
   SPI slave rate. */
#define LPCMASTERCLOCKRATE                  (100000)

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
static void Print_Val16(uint16_t val)
{
	char buf[] = "0xXXXX ";
	buf[5] = "0123456789ABCDEF"[val & 0x0F];
	val >>= 4;
	buf[4] = "0123456789ABCDEF"[val & 0x0F];
	val >>= 4;
	buf[3] = "0123456789ABCDEF"[val & 0x0F];
	val >>= 4;
	buf[2] = "0123456789ABCDEF"[val & 0x0F];
	DEBUGSTR(buf);
}

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

	Chip_SWM_MovablePinAssign(SWM_SPI0_SCK_IO, 12);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, 7);

	Chip_SWM_MovablePinAssign(SWM_SPI0_MOSI_IO, 14);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, 9);
#if defined(BOARD_LPC812MAX)
	Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, 15);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 0);
#else
	Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, 6);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 1);
#endif
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I1);
	Chip_SWM_MovablePinAssign(SWM_SPI0_SSEL_IO, 13);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL_IO, 10);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#elif defined(BOARD_NXP_LPCXPRESSO_824)
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Master Pins for SPI0 */
	Chip_SWM_MovablePinAssign(SWM_SPI0_SSEL0_IO, 15);
	Chip_SWM_MovablePinAssign(SWM_SPI0_SCK_IO, 24);
	Chip_SWM_MovablePinAssign(SWM_SPI0_MISO_IO, 25);
	Chip_SWM_MovablePinAssign(SWM_SPI0_MOSI_IO, 26);

	/* Slave Pins for SPI1 */
	Chip_SWM_MovablePinAssign(SWM_SPI1_SSEL0_IO, 17);
	Chip_SWM_MovablePinAssign(SWM_SPI1_SCK_IO, 18);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MISO_IO, 28);
	Chip_SWM_MovablePinAssign(SWM_SPI1_MOSI_IO, 16);

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);
#else
	/* Configure your own SPI pin muxing here if needed */
#warning "No SPI pin muxing defined"
#endif
}

/* SPI slave select assertion callback function */
static void SPISlaveAssert(SPIS_XFER_T *pSlaveXfer)
{
	/* This function is called when slave is asserted, pSlaveXfer->sselNum
	   contains the asserted slave number (0 - 3) */
}

/* SPI slave send data callback function */
static void SPISlaveSendData(SPIS_XFER_T *pSlaveXfer)
{
	/* This function is called when the transmit buffer is empty and the handler
	   needs more data to send to the master. It may be called multiple times
	   as needed by the slave SPI handler. */

	/* Normally this function would provide a new buffer to the transmit function
	   by updating pSlaveXfer->pTXData and pSlaveXfer->txCount */
}

/* SPI slave receive data callback function */
static void SPISlaveRecvData(SPIS_XFER_T *pSlaveXfer)
{
	/* This function is called when receive buffer is empty and the handler
	   need another buffer to fill.  It may be called multiple times
	   as needed by the slave SPI handler. */

	/* Normally this function would provide a new buffer to the transmit function
	   by updating pSlaveXfer->pRXData and pSlaveXfer->rxCount */
}

/* SPI slave select de-assertion callback function */
static void SPISlaveDeAssert(SPIS_XFER_T *pSlaveXfer)
{
	/* This function is called when the active slave is de-asserted */

	/* For this example, a slave deassertion is assummed to be the end of a
	   transfer */
	sEnd = true;
}

/* SPI slave driver callbacks */
static const SPIS_CALLBACKS_T spiSlaveCallbacks = {
	&SPISlaveAssert,
	&SPISlaveSendData,
	&SPISlaveRecvData,
	&SPISlaveDeAssert
};

/* SPI master select assertion callback function */
static void SPIMasterAssert(SPIM_XFER_T *pMasterXfer)
{
	/* Inidcates tha master just asserted the slave select
	   signal */
}

/* SPI master send data callback function */
static void SPIMasterSendData(SPIM_XFER_T *pMasterXfer)
{
	/* This callback is called when the master needs more data to
	   send. The pMasterXfer->pTXData buffer pointer and transfer size
	   (in items) in pMasterXfer->txCount should be updated. */

	/* If this function sets the pMasterXfer->terminate flag to true,
	   this function won't be called again and the transfer will
	   terminate when the current transmit buffer is complete. */

	/* This example sets up the entire transfer structure without
	   using this callback. */
}

/* SPI master receive data callback function */
static void SPIMasterRecvData(SPIM_XFER_T *pMasterXfer)
{
	/* This callback is called when the master needs another receive
	   buffer. The pMasterXfer->pRXData buffer pointer and transfer size
	   (in items) in pMasterXfer->rxCount should be updated. */

	/* This example sets up the entire transfer structure without
	   using this callback. */
}

/* SPI master select de-assertion callback function */
static void SPIMasterDeAssert(SPIM_XFER_T *pMasterXfer)
{
	/* Inidcates tha master just deasserted the slave select
	   signal */
}

/* SPI master transfer done callback */
static void SPIMasterDone(SPIM_XFER_T *pMasterXfer)
{
	/* Inidcates tha transfer is complete */

	mEnd = true;
}

/* SPI master driver callbacks */
static const SPIM_CALLBACKS_T spiMasterCallbacks = {
	&SPIMasterAssert,
	&SPIMasterSendData,
	&SPIMasterRecvData,
	&SPIMasterDeAssert,
	SPIMasterDone
};

/* Initialize buffers for slave and master */
static uint16_t bufferInit(uint16_t seed)
{
	int i;

	for (i = 0; i < BUFFER_SIZE; i++) {
		seed = seed + 1;

		/* Clear RX buffers, so we know something was received */
		slaveRXBuffer16[i] = masterRXBuffer16[i] = 0;

		/* Seed data for transmit buffers */
		masterTXBuffer16[i] = seed;
		slaveTXBuffer16[i] = ~seed;
	}

	return seed;
}

void showData(char *reg, uint16_t *dat)
{
	int i, j;

	DEBUGSTR("Showing data from :");
	DEBUGSTR(reg);
	DEBUGSTR("\r\n");

	/* Hard coded for a 2x8 16-bit displays */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 8; j++) {
			Print_Val16(*dat);
			dat++;
		}
		DEBUGSTR("\r\n");
	}
}

/* Setup master controller */
static void setupMaster(void)
{
	SPIM_DELAY_CONFIG_T masterDelay;

	/* Initialize SPI controller */
	Chip_SPI_Init(LPC_SPIMASTERPORT);

	/* Call to initialize first SPI controller for mode0, master mode, MSB first */
	Chip_SPI_ConfigureSPI(LPC_SPIMASTERPORT, SPI_MODE_MASTER |	/* Enable master mode */
						  SPI_CLOCK_CPHA0_CPOL0 |	/* Set Clock polarity to 0 */
						  SPI_CFG_MSB_FIRST_EN |/* Enable MSB first option */
						  SPI_CFG_SPOL_LO);	/* Chipselect is active low */

	/* Setup master clock rate, slave clock doesn't need to be setup */
	Chip_SPIM_SetClockRate(LPC_SPIMASTERPORT, LPCMASTERCLOCKRATE);

	/* Setup master delay (all chip selects) */
	masterDelay.PreDelay = 0xD;
	masterDelay.PostDelay = 0xD;
	masterDelay.FrameDelay = 0xD;
	masterDelay.TransferDelay = 0xD;
	Chip_SPIM_DelayConfig(LPC_SPIMASTERPORT, &masterDelay);

	/* For the SPI controller configured in master mode, enable SPI master interrupts
	   for interrupt service. Do not enable SPI_INTENSET_TXDYEN. */
	Chip_SPI_EnableInts(LPC_SPIMASTERPORT, (SPI_INTENSET_RXDYEN |
											SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
											SPI_INTENSET_SSDEN));

	/* Setup master transfer callbacks in the transfer descriptor */
	spiMasterXfer.pCB = &spiMasterCallbacks;
}

/* Setup slave controller */
static void setupSlave(void)
{
	/* Initialize SPI controller */
	Chip_SPI_Init(LPC_SPISLAVEPORT);

	/* Call to initialize first SPI controller for mode0, master mode, MSB first */
	Chip_SPI_ConfigureSPI(LPC_SPISLAVEPORT, SPI_MODE_SLAVE |/* Enable slave mode */
						  SPI_CLOCK_CPHA0_CPOL0 |	/* Set Clock polarity to 0 */
						  SPI_CFG_MSB_FIRST_EN |/* Enable MSB first option */
						  SPI_CFG_SPOL_LO);	/* Chipselect is active low */

	/* Initialize SPI controllers */
	Chip_SPI_Init(LPC_SPISLAVEPORT);

	/* Setup slave controller for 8-bit transfer. Sizes can be altered later
	   for each slave select in the slave select assertion callback once the
	   transfer starts if needed. The master slave transfer size is setup as
	   part of the master transfer description in the options field. */
	Chip_SPI_SetXferSize(LPC_SPISLAVEPORT, 16);

	/* For the SPI controller configured in slave mode, enable SPI slave interrupts
	   for interrupt service. Do not enable SPI_INTENSET_TXDYEN. */
	Chip_SPI_EnableInts(LPC_SPISLAVEPORT, (SPI_INTENSET_RXDYEN |
										   SPI_INTENSET_RXOVEN | SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN |
										   SPI_INTENSET_SSDEN));

	/* Setup slave transfer callbacks in the transfer descriptor */
	spiSlaveXfer.pCB = &spiSlaveCallbacks;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	SPI0 interrupt handler (master)
 * @return	Nothing
 */
void SPIMASTERIRQHANDLER(void)
{
	uint32_t ints = Chip_SPI_GetPendingInts(LPC_SPIMASTERPORT);

	/* Handle SPI slave interrupts only */
	if ((ints & (SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN |
				 SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0) {
		/* SPI slave handler */
		Chip_SPIM_XferHandler(LPC_SPIMASTERPORT, &spiMasterXfer);
	}
}

/**
 * @brief	SPI1 interrupt handler (slave)
 * @return	Nothing
 */
void SPISLAVEIRQHANDLER(void)
{
	uint32_t ints = Chip_SPI_GetPendingInts(LPC_SPISLAVEPORT);

	/* Handle SPI slave interrupts only */
	if ((ints & (SPI_INTENSET_RXDYEN | SPI_INTENSET_RXOVEN |
				 SPI_INTENSET_TXUREN | SPI_INTENSET_SSAEN | SPI_INTENSET_SSDEN)) != 0) {
		/* SPI slave handler */
		errors |= Chip_SPIS_XferHandler(LPC_SPISLAVEPORT, &spiSlaveXfer);
	}
}

/**
 * @brief	Main routine for SPI example
 * @return	Does not return
 */
int main(void)
{
	uint16_t seed = 0;

	SystemCoreClockUpdate();
	Board_Init();

	/* SPI initialization */
	Init_SPI_PinMux();

	/* Initialize stopwatch driver so some event times can be measured */
	StopWatch_Init();

	/* Setup SPI controllers */
	setupMaster();
	setupSlave();

	/* Enable SPI controller interrupts */
	NVIC_EnableIRQ(LPC_SPIMASTERIRQNUM);
	NVIC_EnableIRQ(LPC_SPISLAVEIRQNUM);

	DEBUGSTR("SPI master/slave combined example\r\n");

	/* If you enable loopback mode and connect the master and slave controller's clock
	   and SSEL lines together, the master and slave will wrap data to each other */
	// Chip_SPIM_EnableLoopBack(LPC_SPIMASTERPORT);

	/* Loop forever */
	while (1) {
		/* Setup some data for transmit from master to slave and slave to master */
		seed = bufferInit(seed);

		/* Set slave transfer, this is only the initial transfer,
		   the callbacks can change this later */
		spiSlaveXfer.pTXData16 = slaveTXBuffer16;
		spiSlaveXfer.txCount = sizeof(slaveTXBuffer16) / sizeof(uint16_t);	/* Count is in transfer size */
		spiSlaveXfer.pRXData16 = slaveRXBuffer16;
		spiSlaveXfer.rxCount = sizeof(slaveRXBuffer16) / sizeof(uint16_t);	/* Count is in transfer size */

		/* Set master transfer, this is only the initial transfer,
		   the callbacks can change this later */
		spiMasterXfer.pTXData16 = masterTXBuffer16;	/* Use NULL to send 0x0 */
		spiMasterXfer.txCount = sizeof(masterTXBuffer16) / sizeof(uint16_t);/* Count is in transfer size */
		spiMasterXfer.pRXData16 = masterRXBuffer16;
		spiMasterXfer.rxCount = sizeof(masterRXBuffer16) / sizeof(uint16_t);/* Count is in transfer size */

		/* Setup master transfer options - 16 data bits per transfer, EOT, EOF */
		spiMasterXfer.options =
			SPI_TXCTL_FLEN(16) |		/* This must be enabled as a minimum, use 16 data bits */
			// SPI_TXCTL_EOT |			/* Enable this to assert and deassert SSEL for each individual byte/word, current slave functions for this example do not support this */
			// SPI_TXCTL_EOF |			/* Insert a delay between bytes/words as defined by frame delay time */
			// SPI_TXCTL_RXIGNORE |		/* Enable this to ignore incoming data, or set spiMasterXfer.pRXData16 to NULL to ignore RX data  */
			0;

		/* Transfer will terminate after current buffer is sent. If terminate is not set, the buffers
		   must be setup by the callbacks		*/
		spiMasterXfer.terminate = true;

		/* Use SPI select 0 */
		spiMasterXfer.sselNum = 0;

		/* Time master and slave transfers */
		masterTime = StopWatch_Start();

		/* Limitation: The call below 'pre-buffers' the initial slave transmit datum.
		   If this isn't pre-buffered, a slave transmit underflow will always occur
		   at slave assertion time for the initial transmit datum. The datum sent to the
		   master will be 0. This is ok as we are only using a single slave, but with multiple
		   slaves pre-buffering is not always an option and the master might need to toss the
		   first byte. */
		Chip_SPI_FlushFifos(LPC_SPIMASTERPORT);
		Chip_SPI_FlushFifos(LPC_SPISLAVEPORT);
		Chip_SPIS_PreBuffSlave(LPC_SPISLAVEPORT, &spiSlaveXfer);

		/* Start master transfer */
		Chip_SPIM_Xfer(LPC_SPIMASTERPORT, &spiMasterXfer);

		/* Sleep until transfers are complete */
		mEnd = sEnd = false;
		while ((mEnd == false) || (sEnd == false)) {
			__WFI();
		}

		/* Toggle LED */
		Board_LED_Toggle(0);

		/* Display some information about the transfer */
		Print_Val("\r\nTRANSFER COMPLETE: errors = 0x", errors);
		errors = 0;
		Print_Val("Master total transfer time in uS: 0x", StopWatch_TicksToUs(StopWatch_Elapsed(masterTime)));

		/* Show data */
		showData("Master TX data", masterTXBuffer16);
		showData("Master RX data", masterRXBuffer16);
		showData("Slave  TX data", slaveTXBuffer16);
		showData("Slave  RX data", slaveRXBuffer16);
	}
}
