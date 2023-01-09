#ifndef __USB_CDC_TASK_H
#define __USB_CDC_TASK_H

#include "FreeRTOS.h"
#include "stream_buffer.h"

#define USB_CDC_DATA_EVENT_DATA_RX             (0x01)
#define USB_CDC_DATA_EVENT_CONNECTION_UPDATE   (0x02)
#define USB_CDC_DATA_EVENT_LINE_CODING_UPDATE  (0x04)
#define USB_CDC_DATA_EVENT_ALL  (USB_CDC_DATA_EVENT_DATA_RX            | USB_CDC_DATA_EVENT_CONNECTION_UPDATE | \
                                 USB_CDC_DATA_EVENT_LINE_CODING_UPDATE)

typedef struct
{
    uint8_t cdc_interface_number;
    bool send_eot;
    StreamBufferHandle_t rx_stream;
    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t line_coding_stream;
} usb_cdc_config_t;

void usb_cdc_task_init(usb_cdc_config_t *p_usb_cdc_config);

#endif // __USB_CDC_TASK_H