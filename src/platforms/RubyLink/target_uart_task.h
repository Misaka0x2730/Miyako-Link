#ifndef __USB_UART_TASK_H
#define __USB_UART_TASK_H

#include "FreeRTOS.h"
#include "tusb.h"

#define USB_UART_TASK_TX_COMPLETE        (0x01)

#define TARGET_1_UART_CHANNEL            (0)
#define TARGET_2_UART_CHANNEL            (0)

typedef enum
{
    TARGET_UART_REMOTE_SIDE_USB = 0,
    TARGET_UART_REMOTE_SIDE_TELNET
} target_uart_remote_side;

typedef union
{
    struct
    {
        uint8_t uart_channel;
        target_uart_remote_side remote_side;

        struct
        {
            uint8_t cdc_interface_number;
        } usb_config;

        struct
        {
            uint16_t port;
            cdc_line_coding_t line_coding;
        } telnet_config;
    };
} target_uart_config_t;

void usb_uart_init(void);
void usb_uart_add_interface(target_uart_config_t *p_target_uart_config);

#endif // __USB_UART_TASK_H