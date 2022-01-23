#ifndef LOGGING_COMMON_H
#define LOGGING_COMMON_H

#include "mem_manager.h"
#include "system_func.h"
#include "FreeRTOS.h"
#include "task.h"

#define LIBRARY_LOG_LEVEL       (LOG_INFO)
#define LOG_BUFFER_SIZE         (POOL_SIZE_512)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

extern void System_Log(const char *message, ...);
extern uint64_t System_GetUptime(void);

/*#define SdkLog(level, file, line, message, ...)    do { \
                                                        const uint64_t currentTime = System_GetUptime();                                             \
                                                       char *buffer = MemManager_Alloc(LOG_BUFFER_SIZE);                                            \
                                                       int dataWritten = 0;                                                                         \
                                                       if (buffer != NULL)                                                                          \
                                                       {                                                                                            \
                                                            dataWritten = snprintf(buffer, LOG_BUFFER_SIZE, "%s|%010d.%03d|%s:%d| "message"\r\n",   \
                                                            level, (uint32_t)(currentTime / 1000000), (uint32_t)(currentTime % 1000000) / 1000,     \
                                                            file, line, ##__VA_ARGS__);                                                             \
                                                       }                                                                                            \
                                                       if ((dataWritten > 0) && (dataWritten < LOG_BUFFER_SIZE))                                    \
                                                       {                                                                                            \
                                                           System_SendLogToTask((uint32_t)buffer);                                                  \
                                                       } \
                                                       } while (0)*/

#define SdkLog(level, message, ...)               do {                                                         \
                                                     System_Log_Lock();                                      \
                                                     const uint64_t currentTime = System_GetUptime();        \
                                                     System_Log("%s|%010d.%03d|%s:%d| "message"\r\n", level, \
                                                     (uint32_t)(currentTime / 1000000),                      \
                                                     (uint32_t)(currentTime % 1000000) / 1000,               \
                                                     ##__VA_ARGS__);                                         \
                                                     System_Log_Unlock();                                    \
                                                     } while(0)

/*#define SdkLog(level, file, line, message, ...)    do {const uint64_t currentTime = System_GetUptime();                                            \
                                                       {                                                                                            \
                                                            printf("%s|%010d.%03d|%s:%d| "message"\r\n",   \
                                                            level, (uint32_t)(currentTime / 1000000), (uint32_t)(currentTime % 1000000) / 1000,     \
                                                            file, line, ##__VA_ARGS__);                                                             \
                                                       }                                                                                            \
                                                       } while (0)*/
#endif // LOGGING_COMMON_H