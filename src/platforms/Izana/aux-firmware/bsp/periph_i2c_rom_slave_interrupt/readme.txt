I2C Bus Slave (with Interrupt) Using the ROM API Example
========================================================
 
Example Description
-------------------
This example shows how to configure the I2C port as a slave device
using interrupt mode and the ROM-based APIs.

Notes:
1)	This example requires an I2C connection to an another LPC8xx board.
	Both boards should be connected together on the Arduino connector J1,
	pins PIO14 (SDA) and PIO15 (SCL). Or the LPCXpresso connector, these pins
	are locate on P3-40 (SDA), P3-41 (SCL).

2)	To test the LPC824 or the LPC812-Max as the slave board, use the
	LPCXpresso-812 board as the "master board". The LPCXpresso-812
	does not have pull-ups on the I2C lines. Both the LPC824 and the
	LPC812 MAX board have pull-ups on both I2C lines. This makes them
	unsuitable to work together in this demo. To use the LPCXpresso-812
	as the slave, either the LPC824 or the LPC812-Max are suitable.
	See note 4 on which example to run as the master.

3)	This example extensively uses the DEBUG UART functions To display information
	about the transaction.
	
4)	This example is the slave side of the I2C transaction. To work correctly,
	a second board must be programmed as a master.  The master application can
	be either the periph_i2cm_interrupt or periph_i2cm_polling example.
	
5)	The I2C address: 0xC0 is the same as 0x60 unsed in other examples.
	It is the difference between expressing an I2C address as a 7-bit or 8-bit value.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
Beyond notes #1, and #2 above, there are no special connection requirements for this example.

Board [NXP_812_MAX]:
Beyond notes #1, and #2 above, there are no special connection requirements for this example.
See examples_8xx\LPC812_max-UART.txt for details on configuring the UART.

Board [NXP_LPCXPRESSO_824]:
Beyond notes #1, and #2 above, there are no special connection requirements for this example.

