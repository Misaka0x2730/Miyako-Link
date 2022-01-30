#ifndef __USB_UART_TASK_H
#define __USB_UART_TASK_H

#include "FreeRTOS.h"

#define USB_UART_TASK_TX_COMPLETE        (0x01)

void usb_uart_init(void);
void usb_uart_add_interface(uint8_t channel);

#endif // __USB_UART_TASK_H