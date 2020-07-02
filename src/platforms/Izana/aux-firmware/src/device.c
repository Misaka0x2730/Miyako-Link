#include "device.h"

static void device_process_power(device_cmd cmd);
static void device_process_analog(device_cmd cmd);
static void device_process_ping(device_cmd cmd);
static void device_send_ok(void);
static void device_send_err(void);
static void device_send_pong(void);

void device_process_cmd(device_cmd cmd)
{
	uint8_t result = 0;

	switch(cmd) {
	case DEVICE_CMD_SET_POWER_DISABLE:
	case DEVICE_CMD_SET_POWER_1V8:
	case DEVICE_CMD_SET_POWER_3V3:
	case DEVICE_CMD_SET_POWER_5V:
		device_process_power(cmd);
		break;
	/* Led cmd process */
	case DEVICE_CMD_ENABLE_LED_SER:
		result = target_led_set_state(TARGET_LED_SER, true);
		break;
	case DEVICE_CMD_ENABLE_LED_ACT:
		result = target_led_set_state(TARGET_LED_ACT, true);
		break;
	case DEVICE_CMD_ENABLE_LED_ERR:
		result = target_led_set_state(TARGET_LED_ERR, true);
		break;
	case DEVICE_CMD_DISABLE_LED_SER:
		result = target_led_set_state(TARGET_LED_SER, false);
		break;
	case DEVICE_CMD_DISABLE_LED_ACT:
		result = target_led_set_state(TARGET_LED_ACT, false);
		break;
	case DEVICE_CMD_DISABLE_LED_ERR:
		result = target_led_set_state(TARGET_LED_ERR, false);
		break;

	case DEVICE_CMD_TARGET_RESET:
		target_reset();
	case DEVICE_CMD_TARGET_UNRESET:
		target_unreset();
	case DEVICE_CMD_GET_VREF:
	case DEVICE_CMD_GET_CURRENT:
		device_process_analog(cmd);
		break;
	case DEVICE_CMD_PING:
		device_send_pong();
		break;
	default:
		break;
	}
	if(!result)
		device_send_ok();
	else
		device_send_err();
}

static void device_process_power(device_cmd cmd)
{
	switch(cmd) {
	case DEVICE_CMD_SET_POWER_DISABLE:
		break;
	case DEVICE_CMD_SET_POWER_1V8:
		break;
	case DEVICE_CMD_SET_POWER_3V3:
		break;
	case DEVICE_CMD_SET_POWER_5V:
		break;
	default:
		break;
	}
}

static void device_process_analog(device_cmd cmd)
{
	switch(cmd) {
	case DEVICE_CMD_GET_VREF:
		break;
	case DEVICE_CMD_GET_CURRENT:
		break;
	default:
		break;
	}
}

static void device_send_ok(void)
{

}

static void device_send_err(void)
{

}

static void device_send_pong(void)
{

}