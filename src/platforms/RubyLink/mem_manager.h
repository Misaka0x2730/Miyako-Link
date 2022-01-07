#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H

#include "system_func.h"
#include "queue.h"

typedef struct
{
    size_t size;
    uint8_t data;
} MemManager_Item_t;

typedef struct
{
    size_t bufSize;
    uint32_t bufCount;
    QueueHandle_t queue;

#if MEM_MANAGER_STATS_ENABLED
    struct
  {
    uint32_t allocCnt;
    uint32_t freeCnt;
    uint32_t minFreeCnt;
  } stats;
#endif
} MemManager_Pool_t;

#define POOL_SIZE_16         16
#define POOL_SIZE_64         64
#define POOL_SIZE_128        128
#define POOL_SIZE_256        256
#define POOL_SIZE_512        512
#define POOL_SIZE_1024       1024
#define POOL_SIZE_2048       2048
#define POOL_SIZE_4096       4096

#define MAX_POOL_SIZE        POOL_SIZE_4096

void MemManager_Init(void);

void *MemManager_Alloc(size_t size);

void MemManager_Free(void *pBuffer);

#endif // MEM_MANAGER_H