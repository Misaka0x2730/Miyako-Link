#ifndef __USB_CDC_TASK_H
#define __USB_CDC_TASK_H

#include "FreeRTOS.h"
#include "queue.h"

void usb_cdc_task_init(const uint8_t interface, QueueHandle_t rx_queue, QueueHandle_t tx_queue);

#endif // __USB_CDC_TASK_H