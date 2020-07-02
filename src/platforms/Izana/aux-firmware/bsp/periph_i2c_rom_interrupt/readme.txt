I2C Bus Master with ROM API and Interrupt Example
=================================================

Example Description
-------------------
This example shows how to configure, write, and read, I2C as a bus master in interrupt
mode using the ROM-based APIs.

This demo supports both 7-bit and 10-bit addressing, but only 7-bit addressing is
used in the example. After I2C is setup, the I2C master transmit and receive
functions are called to transmit and receive LED state information.

This demo accesses the LPC812 Max on-board I2C GPIO Expander. This GPIO expander
does not have anything connected to it. It will ACK the I2C transaction, but there will 
be no obvious change on the board. The pin state can be read at IOEXPAN0 which has a
pad at JS5-3.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
Connect the LPCXpresso board onto the LPCXpresso base board.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.
Do not connect an unmodified board to the LPCXpresso baseboard.
The base board will cause the DAP debugger to halt.

Board [NXP_LPCXPRESSO_824]:
Connect the LPCXpresso board onto the LPCXpresso base board.
