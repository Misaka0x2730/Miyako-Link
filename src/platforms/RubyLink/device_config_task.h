#ifndef __DEVICE_CONFIG_TASK_H
#define __DEVICE_CONFIG_TASK_H

#include "general.h"

#define DATA_PIPE_AMOUNT  (PLATFORM_TARGET_INTERFACE_NUM + PLATFORM_USB_UART_INTERFACE_NUM)

void device_config_task_init(void);

#endif // __DEVICE_CONFIG_TASK_H