#ifndef __SYSTEM_FUNC_H
#define __SYSTEM_FUNC_H

#include "FreeRTOS.h"
#include "logging.h"

#define SYSTEM_WAIT_DONT_BLOCK  (0)
#define SYSTEM_WAIT_FOREVER     (portMAX_DELAY)

void System_Init(void);

bool System_IsInInterrupt(void);

void System_Assert(const char* file, const int line);

void System_Log(const char* message, ...);

void System_SendLogToTask(const uint32_t address);

char* System_ReceiveLog(void);

uint64_t System_GetUptime(void);

#endif // __SYSTEM_FUNC_H