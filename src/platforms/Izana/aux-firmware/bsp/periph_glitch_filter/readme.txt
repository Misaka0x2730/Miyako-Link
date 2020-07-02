Digital Glitch-Filter Example
=============================

Example Description
-------------------
This example demonstrates the glitch filter.  There are several things configured
for this example:
1)	The SCT-PWM is configured to run at 10kHz. The output is connected to the
	RED LED -- pin 12 on the LPC824, pin-7 on the LPC812 systems.
2)	The SysTick timer is configured to interrupt every 10mS or 100Hz.
	This interrupt causes the main loop to change the pulse-width by 0.5%.
	This equates to a 1% change every 20ms or a 100% change over 2 seconds.
	The complete pulse-width loop takes 4 seconds from 0% to 100% to 0% again.
3)	Using the switch matrix, the RED LED pin is brought back in through a glitch
	filter running at 600kHz. It is set to look for glitches at least one sample
	wide.  That's about 1.67uS.
4)	When a valid bit is read, the green LED is toggled on, then off.

What you will see is that the RED LED runs through a pulse-width cycle: it gets
brighter, then dimmer to off over a four second loop.  The GREEN LED will come on
as the pulse-width gets wider than the glitch filter configuration (1.67uS).

Note that the glitch filter works on both ends of the range: if the low part of
PWM wave is less than 1.67uS or the high part of the PWM wave is less than 1.67uS.

Notes:
1)	When using the LPCXpresso base-board the debug UART loses several characters
	at the start of each transmission. This is due to C20 on the base-board.
	The issue can be resolved by removing C20.

2)	The LPCXpresso base-board does not require power to run this example.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
Connect the LPCXpresso board onto the LPCXpresso base board.

Board [NXP_812_MAX]:
Connect the LPCXpresso board onto the LPCXpresso base board.

Board [NXP_LPCXPRESSO_824]:
Connect the LPCXpresso board onto the LPCXpresso base board.

