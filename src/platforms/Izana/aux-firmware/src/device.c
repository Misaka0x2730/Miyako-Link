#include "device.h"

static void device_send_vref(void);
static void device_send_current(void);
static void device_send_ok(void);
static void device_send_err(void);
static void device_send_pong(void);

static volatile bool device_tx_flag = false;

static void device_set_tx_flag(void);
static void device_send_response(uint8_t data);

void device_init(void)
{
	target_init();
	
	Chip_UART_Init(LPC_USART0);
	Chip_SWM_MovablePinAssign(SWM_U0_RXD_I, UART_RX_PIN);
	Chip_SWM_MovablePinAssign(SWM_U0_TXD_O, UART_TX_PIN);
	//Chip_SWM_MovablePinAssign(SWM_U0_SCLK_IO, UART_CLK_PIN);

	Chip_UART_ConfigData(LPC_USART0,	UART_CFG_DATALEN_8 | 
										UART_CFG_PARITY_NONE |
										UART_CFG_STOPLEN_1);
	Chip_Clock_SetUARTClockDiv(1);
	Chip_Clock_SetUSARTNBaseClockRate(UART_BAUD_RATE, true);
	Chip_UART_TXEnable(LPC_USART0);
	Chip_UART_IntEnable(LPC_USART0, UART_INTEN_RXRDY);
	Chip_UART_Enable(LPC_USART0);
	//NVIC_EnableIRQ(UART0_IRQn);
}

void device_process_cmd(device_cmd cmd)
{
	uint8_t result = 0;

	switch(cmd) {
	case DEVICE_CMD_SET_POWER_DISABLE:
		target_power_disable();
		break;
	case DEVICE_CMD_SET_POWER_1V8:
		target_power_disable();
		target_power_1v8();
		target_power_enable();
		break;
	case DEVICE_CMD_SET_POWER_3V3:
		target_power_disable();
		target_power_3v3();
		target_power_enable();
		break;
	case DEVICE_CMD_SET_POWER_5V:
		target_power_disable();
		target_power_5v();
		target_power_enable();
		break;
	case DEVICE_CMD_RESET_OVER_CURRENT:
		target_clear_over_current_flag();
		target_power_enable();
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
		break;
	case DEVICE_CMD_TARGET_UNRESET:
		target_unreset();
		break;
	case DEVICE_CMD_GET_VREF:
		device_send_vref();
		return;
	case DEVICE_CMD_GET_CURRENT:
		device_send_current();
		return;
	case DEVICE_CMD_PING:
		device_send_pong();
		return;
	default:
		break;
	}
	if(!result)
		device_send_ok();
	else
		device_send_err();
}

static void device_send_vref(void)
{

}

static void device_send_current(void)
{
	
}

static void device_send_ok(void)
{
	device_send_response(RESPONSE_CMD_OK);
}

static void device_send_err(void)
{
	device_send_response(RESPONSE_CMD_ERR);
}

static void device_send_pong(void)
{
	device_send_response(RESPONSE_CMD_PONG);
}

static void device_set_tx_flag(void)
{
	device_tx_flag = true;
}

bool device_get_tx_flag(void)
{
	return device_tx_flag;
}

void device_clear_tx_flag(void)
{
	device_tx_flag = false;
}

bool device_get_over_current_status(void)
{
	return target_get_over_current_flag();
}

void device_process_over_current(void)
{
	device_send_response(RESPONSE_CMD_OVER_CURRENT);
}

bool device_get_adc_status(void)
{
	return target_get_adc_flag();
}

void device_process_adc(void)
{
	target_calculate_vref_and_current();
}

static void device_send_response(uint8_t data)
{
	Chip_UART_SendBlocking(LPC_USART0, &data, 1);
}

uint8_t device_read_cmd()
{
	uint8_t cmd = 0;
	if(Chip_UART_Read(LPC_USART0, &cmd, 1))
		return cmd;
	else
		return 0;
}