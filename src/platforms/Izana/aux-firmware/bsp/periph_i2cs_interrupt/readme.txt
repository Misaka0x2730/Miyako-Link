I2CS Bus Slave Example using Interrupt Mode
===========================================

This example shows how to configure I2C as a bus slave in interrupt mode using
the I2CS driver.

This example provides 2 simple (emulated) EEPROMs at different I2C slave
addresses. Both are on the same I2C bus, but the slave controller will be
configured to support 2 slave addresses on the single bus. The emulated
EEPROMs have their memory locations set and read via I2C write and read
operations. Operations can be as little as a byte or continuous until the
master terminates the transfer. The following operations are supported:- <START> <ADDR><W> Write 16-bit address <STOP>
- <START> <ADDR><W> Write 16-bit address <REPEAT START><R> READ READ ... READ <STOP> (unbound read)
- <START> <ADDR><W> Write 16-bit address WRITE WRITE ... WRITE <STOP> (unbound write)
- <START> <ADDR><R> READ READ ... READ <STOP> (unbound read)
Note: Slave address is 0x28.

Unbound read operations have no limit on size and will go as long as the master
requests or sends data. If the end of emulated EEPROM is reached, the EEPROM address
will wrap. All reads and write operations auto-increment. Read operations without a
16-bit address will use the last incremented address.

The I2C slave processing is handled entirely in the I2C slave interrupt handler.
This example doesn't use the Chip_I2CS_Xfer() function; it implements the slave
support inside the I2C interrupt handler in real-time.

The example also provides the master interface on the same I2C bus as the slave
to communicate the the emulated EEPROMs without requiring an external I2C master.

Note: this example uses the DEBUG UART to show the status of the read/write tests.

Special Connection Requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.
See examples_8xx\LPC812_max-UART.txt for details on configuring the UART.

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.
