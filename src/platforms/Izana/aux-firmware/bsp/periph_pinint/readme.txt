Pin Interrupt Example
=====================

Example Description
-------------------
The Pin interrupt example demonstrates using the pin interrupt API functions
pin interrupts & wake up from low power sleep state using the pin
interrupts.

This example configures the pin interrupt channel 7 as falling edge 
wake up interrupt. The interrupt channel 7 is connected to GPIO pin 
PIO0-0 in GPIO block. The example will go to sleep mode. To wake up from 
sleep mode, provide a falling edge pulse on the pin used for wakeup.

On NXP LPCXpresso-812 board, PIO0-0 is connected to potentiometer which 
can be used to provide the falling edge pulse. The application will go to 
sleep state in a loop until the potentiometer is turned enough to generate
a falling edge. With every wake up the board, LED0 will be toggled.

On LPC812-MAX board the RESET switch SW1 connected to PIO0_5 is used to
provide the falling edge pulse. The application will go to sleep state in
a loop until reset is pressed. With every wake up the board, the red will
be toggled.

On LPC824 Max board the WAKE switch SW1 connected to PI00_4 is used to 
provide the falling edge pulse.  The application will go to sleep state in
a loop until the WAKE button is pressed.  Upon waking, the red LED will 
be toggled.

Special Connection Requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.
Use the potentiometer to activate the interrupt.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.
Use SW1 (Wake) button to activate the interrupt.

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.
On LPC824 Max board use SW1 (WAKE) button to activate the interrupt.

