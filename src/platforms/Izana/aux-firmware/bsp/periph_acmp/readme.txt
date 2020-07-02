Analog Comparator Example
=========================

Example Description
-------------------
The ACMP example demonstrates the analog comparator.

This example configures the comparator positive input to the  potentiometer
LPCXpresso base-board (NXP_LPCXPRESSO_824) or on-board potentiometer
(NXP_LPCXPRESSO_812). The negative input is connected to the internal band-gap
reference voltage. The output of the comparator is used to drive an LED.
The comparator threshold is configured to generate an interrupt.

The NXP_812_MAX board requires an external potentiometer to run this
demo program.

When the potentiometer is adjusted up and down, the voltage to the positive 
side of the comparator is changed. When the voltage crosses the negative
comparator input, a CMP_IRQ is fired. Based on the voltage reference in relation
to the band-gap, the LED state will change.

This example does not use the RS-232 port.

Special Connection Requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.
The LPCXpresso 812 board has an on-board potentiometer.  This is connected to
ACMP-I1 (pin 19). The example uses this to drive the positive side of the comparator.

Board [NXP_812_MAX]:
The 812 MAX board expects analog input on pin-19 (PIO0_0/ACMP_I1/TDO). The processor
pin connects to P2-10 on the LPCXpresso connector and pin J2-8 on the Arduino terminals.
To use this demo, connect a 100k linear potentiometer to the following Arduino pins:
	Potentiometer	Arduino		Notes
		T1 (end-1)	J8-4		3.3V
		T2 (wiper)	J2-8		comparator input
		T3 (end-2)	J8-7		GND

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.
The LPCXpresso 824 board expects analog input on pin-23 (P0_6/ADC1/VDDCMP). The demo is 
designed to use the LPCXpresso base-board without any special wiring or jumpers.
The processor pin connects to P3-15 on the LPCXpresso connector and pin J5-1 on
the Arduino terminals. Note however, that base-board J-27 must be installed.
This jumper connects the potentiometer (R105) to J4-15 on the LPCXpresso connector.

