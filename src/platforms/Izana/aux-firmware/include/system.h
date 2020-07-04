#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#include "chip.h"

#define MAIN_FREQ	(48000000UL)
#define SYS_FREQ	(24000000UL)
#define IOCON_DIV	(192)			//250kHz

void SystemInit(void);

#endif /* __SYSTEM_H_ */