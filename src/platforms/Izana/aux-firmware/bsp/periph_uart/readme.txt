UART Test with LEDs
========================================

Example Description
-------------------
The UART example shows how to use the UART.

To use the example, connect a serial cable to the board's RS232/UART port and
start a terminal program to monitor the port.  The terminal program on the host
PC should be setup for 115200-8-N-1.
Once the example is started, a brief message is sent to the terminal. Any data
received will be displayed on the terminal.

Special connection requirements
======================================================================================
Board [NXP_LPCXPRESSO_812]:
Board [NXP_812_MAX]:


Method #1:
UART:	LPC_UART0
PINS:	PIO0_4 (TX) / PIO0_0 (RX)
Output: Arduino connector

Procedure:

1.	Board connection:
	Connect TX, RX and GND from J2 [Arduino Digital Header] to an FTDI Cable.
	LPC812            FTDI cable
	 GND: J1-4         black
	 RX:  J2-8 (D0)    orange
	 TX:  J2-7 (D1)    yellow

2.	Source code changes:
	Do one of the following:
	1) In the IDE, set "UART_ARDUINO" as a pre-processor define.
	2) in project lib_lpc_boards_8xx, file board.h near line 59, add a define for UART_ARDUINO.

3.	Re-build the code

4.	Connect to the serial port via terminal emulator (TeraTerm, PuTTY, etc.)


Method #2:
UART:	LPC_UART0
PINS:	PIO0_6 (TX) / PIO0_1 (RX)
Output: Host USB connector (J3)
Procedure:

1.	Modify the board:
	SJ4:	1) Remove the 0-ohm resistor between TARGET_RX-PIO0_1 and IOEXP_INTR (2)
			2) Place a solder bridge between TARGET_RX-PIO0_1 and TARGET_RX (1)
	SJ1:	1) Remove the 0-ohm resistor between TARGET_TX-PIO0_6 and PIO0_6 (2)
			2) Install a solder bridge between TARGET_TX-PIO0_6 and TARGET_TX (1)


2.	Connect the board to the host.
	This is done by connecting J3 (USB micro connector) to the host.
	A serial port will be enumerated something like "mbed Serial Port (COM3)".

3.	Source code changes:
	Verify in the IDE, that "UART_ARDUINO" is NOT set as a pre-processor define.  -- default
	Verify in project lib_lpc_boards_8xx, file board.h near line 59, that UART_ARDUINO is NOT defined

4.	Re-build the code

5.	Connect to the serial port via terminal emulator (TeraTerm, PuTTY, etc.)


For reference, here's the source code from board.c in project lib_lpc_boards_8xx
#ifdef	UART_ARDUINO
	/* Connect the U0_TXD_O and U0_RXD_I signals to port pins: P0.4 (TX), P0.0 (RX) */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I1);
	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 4);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 0);
#else
	/* Connect the U0_TXD_O and U0_RXD_I signals to port pins: P0.6 (TX), P0.1 (RX) */
	Chip_SWM_DisableFixedPin(SWM_FIXED_ACMP_I2);
	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, 6);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, 1);
#endif

======================================================================================

Board [NXP_LPCXPRESSO_824]:
There are two different ways that the UART is used on the LPC824_xpresso boards:
1) The debug port is configured to use the USB host link.
2) Several UART specific examples use the Arduino connection instead of the USB
host link. When they do, the USB host link is disabled.  These examples are:
periph_uart_rb, periph_uart_rom_int, and periph_uart_rom_polling.

Note: Unlike the LPC812_MAX no hardware changes are required to use either of the UARTs.

Method #1: HSB host link
UART:	LPC_UART1
PINS:	TARGET_TX-P0_7 (TX) / TARGET_RX-P0_18 (RX)
Output: Host USB connector (J3)
Procedure:
1.	Connect the board to the host.
	This is done by connecting J3 (USB micro connector) to the host.
	A serial port will be enumerated something like "mbed Serial Port (COM3)".

2.	Connect to the serial port via terminal emulator (TeraTerm, PuTTY, etc.)

Method #2: Arduino connection
UART:	LPC_UART1
PINS:	PIO0_4 (TX) / PIO0_0 (RX)
Output: Arduino connector

Procedure:
1.	Board connection:
	Connect TX, RX and GND from J2 [Arduino Digital Header] to an FTDI Cable.
	LPC812            FTDI cable
	 GND: J1-4         black
	 RX:  J2-8 (D0)    orange
	 TX:  J2-7 (D1)    yellow

2.	Connect to the serial port via terminal emulator (TeraTerm, PuTTY, etc.)


