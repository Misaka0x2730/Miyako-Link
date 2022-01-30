#ifndef __USB_CDC_TASK_H
#define __USB_CDC_TASK_H

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "queue.h"

#define USB_CDC_DATA_EVENT_DATA_RX             (0x01)
#define USB_CDC_DATA_EVENT_CONNECTION_UPDATE   (0x02)
#define USB_CDC_DATA_EVENT_LINE_CODING_UPDATE  (0x04)
#define USB_CDC_DATA_EVENT_ALL  (USB_CDC_DATA_EVENT_DATA_RX            | USB_CDC_DATA_EVENT_CONNECTION_UPDATE | \
                                 USB_CDC_DATA_EVENT_LINE_CODING_UPDATE)

void usb_cdc_task_init(const uint8_t interface, const char* rx_thread_name, const char* tx_thread_name, QueueHandle_t rx_queue, QueueHandle_t tx_queue);
void usb_cdc_test_task_init(const uint8_t interface, const char* rx_thread_name, const char* tx_thread_name, StreamBufferHandle_t rx_stream, StreamBufferHandle_t tx_stream, StreamBufferHandle_t line_coding_stream);

#endif // __USB_CDC_TASK_H