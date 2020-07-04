#include "system.h"

const uint32_t OscRateIn = 0;
const uint32_t ExtRateIn = 0;

void SystemInit(void)
{
	Chip_IRC_SetFreq(MAIN_FREQ, SYS_FREQ);

	Chip_SWM_Init();

	Chip_Clock_SetIOCONCLKDIV(IOCONCLKDIV0, IOCON_DIV);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
}