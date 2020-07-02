#include "system.h"

const uint32_t OscRateIn = 0;
const uint32_t ExtRateIn = 0;

void SystemInit(void)
{
	Chip_IRC_SetFreq(MAIN_FREQ, SYS_FREQ);
}