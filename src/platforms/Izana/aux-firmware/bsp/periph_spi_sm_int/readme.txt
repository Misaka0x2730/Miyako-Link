LPC8xx SPI Master / Slave Example using SPIM and SPIS Drivers
=============================================================

Example Description
-------------------
This example configures two SPI ports: SPI0 and SPI1. They must be wired together
to work properly.

The application configures the I/O to match the different chip/board architectures.
It supports all three chip/board combinations.

It uses the RS-232 debug port.

Special Connection Requirements
-------------------------------
Board [NXP_LPCXPRESSO_812]:
	Signal	SPI0-pin/mbed		SPI1-pin/mbed			Comment
	MOSI		P.14 / J6-5			P.09 / J6-17
	MISO		P.06 / J6-6			P.01 / J6-16
	SCK			P.12 / J6-7			P.07 / J6-18		RED LED
	SSEL		P.13 / J6-8			P.10 / J6-40

Board [NXP_812_MAX]:
	Signal	SPI0-pin/Arduino	SPI1-pin/Arduino		Comment
	SCK			P0.12 / J1-5		P0.07 / J2-1		RED LED
	MISO		P0.15 / J1-6		P0.00 / J2-8
	MOSI		P0.14 / J1-7		P0.09 / J2-4
	SSEL		P0.13 / J1-8		P0.10 / J1-2

Board [NXP_LPCXPRESSO_824]:
	Signal	SPI0-pin/Arduino	SPI1-pin/Arduino		Comment
	SCK			P0.24 / J1-5		P0.18 /J2-4
	MISO		P0.25 / J1-6		P0.28 /J2-3
	MOSI		P0.26 / J1-7		P0.16 /J2-2			GREEN LED
	SSEL		P0.15 / J1-8		P0.17 /J2-1

