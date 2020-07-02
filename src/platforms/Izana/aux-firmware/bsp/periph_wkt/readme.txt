Self Wake-up Timer (WKT) Example
================================

Example Description
-------------------
The WKT example demonstrates using the Wake-up timer to wake-up the MCU from
low power states. The wakeup timer is configured for different wakeup times
using the WKT clock sources. The chip is then placed into a low power mode
and will wakeup from the WKT timer event. PMU power modes tested are:
	MCU_SLEEP,
	MCU_DEEP_SLEEP,
	MCU_POWER_DOWN,
	MCU_DEEP_POWER_DOWN
The LED on the board will toggle for each wakeup event.
Expect to lose your debugger connection with this example when the MCU
goes into low power states.</i>

Notes:
1)	This example does not use the serial port.

2)	It is possible to comment any of the low power state calls.  For example,
	it is possible to comment out all of the calls except for the deep power-down call.
	
3)	It is possible to change the delay in the WakeupTest() calls to delay longer or shorter.
	A shorter delay will require adjustments to WakeupTest() as it is designed to operate
	on one-second boundaries.

4)	When in a low-power state, it's difficult to program the LPC812 / LPC824.
	Connect PIO0-12 to ground to enable the ISP. The device can then be programmed.

Special connection requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.
