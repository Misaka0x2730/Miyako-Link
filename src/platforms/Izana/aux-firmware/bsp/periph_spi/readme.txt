SPI Example
===========

Example Description
-------------------
This example describes how to use SPI in POLLING or INTERRUPT mode.
The LOOPBACK_TEST macro is used to enable Loop-back mode of SPI peripheral. It allows
a simple software testing. The transmit and receive data is connected together. No
more connection is required in this case.
If LOOPBACK_TEST macro is disabled, it is needed to connect 2 hardware boards,
one for Master and one for Slave.
     SPI configuration:
         - CPHA = 0: Data is sampled on the first clock edge of SCK.
         - CPOL = 0: The rest state of the clock (between frames) is low.
         - Sample rate = 100KHz.
         - DSS = 8: 8 bits per transfer.
     After initialize transmit buffer, SPI master/slave will transfer a number of bytes
     to SPI slave/master and receive data concurrently.
     After a transfer completed, receive and transmit buffer will be compared. If the receive
     buffer is matched with transmit buffer, the Blue led blinks. If not the Red led blinks.
     This example supports 2 transfer modes: POLLING mode and INTERRUPT mode.

 - Configure hardware, connect master board and slave board as below
 - Update SPI_MODE_TEST macro for the relevant board, Build and Program Internal Flash.
 - Reset the slave board and then master board. Observe the LED on board.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:

Board [NXP_812_MAX]:

Board [NXP_LPCXPRESSO_824]:
Hardware configuration
 - SPI pins:
		- P0.12: SPI_SCK
		- P0.13: SPI_SSEL
	    - P0.6: SPI_MISO
     - P0.14: SPI_MOSI
		The SCK pin of master board is connected to SCK of slave board.
		The SSEL pin of master board is connected to SSEL of slave board.
		The MISO pin of master board is connected to MISO of slave board.
		The MOSI pin of master board is connected to MOSI of slave board.
     Common ground must be connected together between two boards.
