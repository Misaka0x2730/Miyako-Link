#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "target.h"

typedef enum {
	DEVICE_CMD_SET_POWER_DISABLE = 0,
	DEVICE_CMD_SET_POWER_1V8,
	DEVICE_CMD_SET_POWER_3V3,
	DEVICE_CMD_SET_POWER_5V,
	DEVICE_CMD_ENABLE_LED_SER,
	DEVICE_CMD_ENABLE_LED_ACT,
	DEVICE_CMD_ENABLE_LED_ERR,
	DEVICE_CMD_DISABLE_LED_SER,
	DEVICE_CMD_DISABLE_LED_ACT,
	DEVICE_CMD_DISABLE_LED_ERR,
	DEVICE_CMD_TARGET_RESET,
	DEVICE_CMD_TARGET_UNRESET,
	DEVICE_CMD_GET_VREF,
	DEVICE_CMD_GET_CURRENT,
	DEVICE_CMD_PING
} device_cmd;

typedef enum {
	RESPONSE_CMD_VREF_CURRENT = 0,
	RESPONSE_CMD_OK,
	RESPONSE_CMD_OVER_CURRENT,
	RESPONSE_CMD_PONG
} response_cmd;

void device_process_cmd(device_cmd cmd);

#endif /* __DEVICE_H_ */