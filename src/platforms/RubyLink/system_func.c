#include <stdio.h>
#include <stdarg.h>

#include "system_func.h"
#include "mem_manager.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "task.h"

//#define LOG_BUFFER_SIZE     (POOL_SIZE_1024)

static QueueHandle_t logQueue;

void System_Init(void)
{
    logQueue = xQueueCreate(10, sizeof(uint32_t));
}

bool System_IsInInterrupt(void)
{
    return taskCHECK_IF_IN_ISR();
}

void System_Assert(const char* file, const int line)
{
    //__BKPT(0);
}

void System_Log(const char* message, ...)
{
    va_list args;

    va_start(args, message);
    char *buffer = MemManager_Alloc(LOG_BUFFER_SIZE);
    int dataWritten = 0;
    if (buffer != NULL)
    {
        dataWritten = vsnprintf(buffer, LOG_BUFFER_SIZE, message, args);
    }
    //vprintf(message, args);
    va_end(args);

    if ((dataWritten > 0) && (dataWritten < LOG_BUFFER_SIZE))
    {
        System_SendLogToTask((uint32_t)buffer);
    }
}

uint64_t System_GetUptime(void)
{
    return time_us_64();
}

void System_SendLogToTask(const uint32_t address)
{
    const char *dataPtr = (char*)(address);

    BaseType_t result = xQueueSendToFront(logQueue, &address, SYSTEM_WAIT_DONT_BLOCK);

    if (result != pdTRUE)
    {
        MemManager_Free((void*)dataPtr);
    }
}

char* System_ReceiveLog(void)
{
    uint32_t address = 0;

    BaseType_t result = xQueueReceive(logQueue, &address, SYSTEM_WAIT_FOREVER);

    if (result != pdTRUE)
    {
        return NULL;
    }

    return (char*)(address);
}