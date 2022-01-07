#include "tusb.h"
#include "FreeRTOS.h"
#include "usb_task.h"

static void usb_task_thread(void *param);

void usb_task_init(void)
{
    TaskHandle_t usb_task;
    BaseType_t status = xTaskCreate(usb_task_thread,
                         "usb_task",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         configMAX_PRIORITIES - 1,
                         &usb_task);

    vTaskCoreAffinitySet(usb_task, 0x01);
}

static void usb_task_thread(void *param)
{
    (void) param;

    tusb_init();

    while (1)
    {
        tud_task();
    }
}