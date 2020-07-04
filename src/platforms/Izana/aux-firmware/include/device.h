#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "target.h"

typedef enum {
	DEVICE_CMD_SET_POWER_DISABLE = 1,
	DEVICE_CMD_SET_POWER_1V8,
	DEVICE_CMD_SET_POWER_3V3,
	DEVICE_CMD_SET_POWER_5V,
	DEVICE_CMD_RESET_OVER_CURRENT,
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
	DEVICE_CMD_PING,
	DEVICE_CMD_DUMB = 0xFF
} device_cmd;

typedef enum {
	RESPONSE_CMD_OK = 0xFA,
	RESPONSE_CMD_ERR = 0xFB,
	RESPONSE_CMD_OVER_CURRENT = 0xFC,
	RESPONSE_CMD_PONG = 0xFD
} response_cmd;

#define UART_RX_PIN	  	(0x08)
#define UART_TX_PIN	  	(0x09)
#define UART_CLK_PIN  	(0x0E)
#define UART_BAUD_RATE	(115200*16)

void device_init(void);
void device_process_cmd(device_cmd cmd);
bool device_get_tx_flag(void);
void device_clear_tx_flag(void);
bool device_get_over_current_status(void);
void device_process_over_current(void);
bool device_get_adc_status(void);
void device_process_adc(void);
uint8_t device_read_cmd();
#endif /* __DEVICE_H_ */