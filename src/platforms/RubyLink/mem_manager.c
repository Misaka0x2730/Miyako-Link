#include <string.h>

#include "mem_manager.h"
#include "semphr.h"

static const struct
{
    size_t bufSize;
    uint32_t bufCount;
    char *poolName;
}  MEM_MANAGER_POOL_CONFIG[] =
{
    { .bufSize = POOL_SIZE_16, .bufCount =  200,  .poolName = "Pool 16 bytes"   },
    {            POOL_SIZE_64,              100,              "Pool 64 bytes"   },
    {            POOL_SIZE_128,             50,               "Pool 128 bytes"  },
    {            POOL_SIZE_256,             10,               "Pool 256 bytes"  },
    {            POOL_SIZE_512,             10,               "Pool 512 bytes"  },
    {            POOL_SIZE_1024,            10,               "Pool 1024 bytes" },
    {            POOL_SIZE_2048,            3,                "Pool 2048 bytes" },
    {            POOL_SIZE_4096,            3,                "Pool 4096 bytes" }
};

#define POOL_AMOUNT (sizeof(MEM_MANAGER_POOL_CONFIG)/sizeof(MEM_MANAGER_POOL_CONFIG[0]))

static MemManager_Pool_t memManagerPools[POOL_AMOUNT];
static SemaphoreHandle_t memManagerMutex;

void MemManager_Init(void)
{
    memManagerMutex = xSemaphoreCreateMutex();

    for (size_t i = 0; i < POOL_AMOUNT; i++)
    {
        memManagerPools[i].bufSize  = MEM_MANAGER_POOL_CONFIG[i].bufSize;
        memManagerPools[i].bufCount = MEM_MANAGER_POOL_CONFIG[i].bufCount;
        memManagerPools[i].queue = xQueueCreate(memManagerPools[i].bufCount, sizeof(MemManager_Item_t*));

        MemManager_Item_t *pItem;
        for (size_t j = 0; j < memManagerPools[i].bufCount; j++)
        {
            pItem = pvPortMalloc(sizeof(MemManager_Item_t) + (MEM_MANAGER_POOL_CONFIG[i].bufSize - 1));
            pItem->size = MEM_MANAGER_POOL_CONFIG[i].bufSize;
            BaseType_t result = xQueueSendToFront(memManagerPools[i].queue, &pItem, SYSTEM_WAIT_DONT_BLOCK);
        }
    }
}

void *MemManager_Alloc(size_t size)
{
    if (System_IsInInterrupt())
    {
        //TODO: assert
        return NULL;
    }

    MemManager_Item_t *pItem = NULL;
    BaseType_t result;

    /*if (xSemaphoreTake(memManagerMutex, SYSTEM_WAIT_DONT_BLOCK) != pdTRUE)
    {
        return NULL;
    }*/

    for (size_t i = 0; i < POOL_AMOUNT; i++)
    {
        if (size <= memManagerPools[i].bufSize)
        {
            result = xQueueReceive(memManagerPools[i].queue, &pItem, SYSTEM_WAIT_DONT_BLOCK);

            if (result == pdTRUE)
            {
                memset(&(pItem->data), 0, pItem->size);
                xSemaphoreGive(memManagerMutex);
                return &(pItem->data);
            }
            else
            {
                break;
            }
        }
    }
    //xSemaphoreGive(memManagerMutex);
    return NULL;
}

void MemManager_Free(void *pBuffer)
{
    if (System_IsInInterrupt())
    {
        return;
    }

    if (pBuffer == NULL)
    {
        return;
    }

    MemManager_Item_t *pItem = (MemManager_Item_t*)(pBuffer - sizeof(size_t));

    /*if (xSemaphoreTake(memManagerMutex, SYSTEM_WAIT_DONT_BLOCK) != pdTRUE)
    {
        return;
    }*/

    for (size_t i = 0; i < POOL_AMOUNT; i++)
    {
        if (pItem->size == memManagerPools[i].bufSize)
        {
            BaseType_t result = xQueueSendToFront(memManagerPools[i].queue, &pItem, SYSTEM_WAIT_DONT_BLOCK);

            break;
        }
    }

    //xSemaphoreGive(memManagerMutex);
}