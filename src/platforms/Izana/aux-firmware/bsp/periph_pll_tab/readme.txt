PLL / IRC table Test Example
============================

Example Description
-------------------
This example tests the Chip_IRC_SetFreq() function.  This function is driven by a table of 71 possible combinations.
All combinations are tested.

At boot, a log-on message is displayed.  Then a state machine is called to initialize the next set of frequencies.
Each table entry goes through the same process:
1) Select the main / system clock combinataion. 
2) Configure the clock and UART for this configuration.
3) Display the configuration on the UART.
4) Set the LED to red.  This indicates that the main clock is being sent to CLKOUT.
5) Sleep for 3 seconds.
6) Set the LED to green.  Change CLKOUT divider to match the system clock.
7) Sleep for 3 seconds.
8) Shut-off CLKOUT entirely; bump to the next table entry.
9) Sleep for 3 seconds.


Special Connection Requirements
-------------------------------
CLKOUT is configured to output on Arduino connector J1 pin 5.  To see the clock output, connect an oscilloscope to pins 4 (ground) and 5 (CLKOUT)
on the Arduino digital connection (J1).
