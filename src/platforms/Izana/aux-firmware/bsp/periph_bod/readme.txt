Brown-out Detector Example
==========================

Example Description
-------------------
The brown-out example shows how to use the brown-out detector (BOD)
on the LPC8XX.The BOD is setup to generate a BOD interrupt when
power is lost. The interrupt will attempt to toggle the LED on as
power is lost.

To use this example, build and program it and then run it on the board.
Power off the board by removing the power (USB) cable. As the power is
declining on the board, the LED will toggle on (quickly) and then turn
off as power is lost.

Special Connection Requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
There are no special connection requirements for this example.

Board [NXP_812_MAX]:
There are no special connection requirements for this example.

Board [NXP_LPCXPRESSO_824]:
There are no special connection requirements for this example.

