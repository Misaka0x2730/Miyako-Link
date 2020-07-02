I2C Bus Master (interrupt) ROM API Example
==========================================

Example Description
-------------------
This example shows how to configure I2C as a bus master in interrupt mode using
the I2CM driver. This example uses 7-bit addressing to write LEDs on the LPCXpresso
base-board. After I2C is setup, the I2CM master transmit functions are called
through the i2cm_8xx driver routines.

Notes:
1)	This example does not use the RS-232 port.

2)	The LPCXpresso base-board does not require power to run this example.

3)	The I2C address: 0x60 is the same as 0xC0 as used in other examples.
	It is the difference between expressing an I2C address as a 7-bit or 8-bit value.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:

Board [NXP_812_MAX]:
This demo accesses the LPC812 Max on-board I2C GPIO Expander. This GPIO expander
does not have anything connected to it. It will ACK the I2C transaction, but there will 
be no obvious change on the board. The pin state can be read at IOEXPAN0 which has a
pad at JS5-3.

Board [NXP_LPCXPRESSO_824]:
This example requires an LPCXpresso base-board to run.

