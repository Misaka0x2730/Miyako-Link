IAP FLASH programming & FLASH signature example using IAP commands 
to write to FLASH memory with FLASH signature generator 
=======================================================

Example Description
-------------------
The IAP example demonstrates programming a FLASH block during run-time and
generating a FLASH signature. For this example, the code is running from FLASH
and a FLASH block not used for the executing code will be erased and programmed
with some data. After the program is complete, the signature for the programmed
area is generated. The example also toggles the LED in the systick interrupt.
The interrupts need to be disabled during the IAP calls that change FLASH data
and re-enabled after the calls are complete.<br>

Do not run this example too many times or set it up to repeatedly erase and
reprogram FLASH as it will wear out FLASH.

This example uses the Debug UART.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.
The debug UART output can be seen by plugging an FTDI cable into J7.
Use FTDI cable: C232HD-DDHSP-0. Do not use FTDI cable TTL-232R-3V3,
since VCC out is +5V on this cable.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.
See examples_8xx\LPC812_max-UART.txt for details on configuring the UART.

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.
The debug UART works through the DAP debugger USB connection.

