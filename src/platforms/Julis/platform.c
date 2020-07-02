/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file implements the platform specific functions for the STM32
 * implementation.
 */

#include "general.h"
//#include "cdcacm.h"
//#include "usbuart.h"
#include "morse.h"
#include "platform.h"
#include "gdb_if.h"

static const gpio_t pin_list[INTERFACE_NUMBER][MAX_INTERFACE_PINS] = {
	{
		GPIO_PIN(PORT_G, 1), 	//INTERFACE: IDLE LED
		GPIO_PIN(PORT_F, 9), 	//INTERFACE: ACT LED
		GPIO_PIN(PORT_F, 7), 	//INTERFACE: ERR LED

		GPIO_PIN(PORT_G, 3),	//SWD: RST
		GPIO_PIN(PORT_C, 12),  	//SWD: SWDIO
		GPIO_PIN(PORT_D, 2),	//SWD: SWCLK
		0xFFFFFFFF,				//JTAG: RST
		0xFFFFFFFF, 			//JTAG: TRST
		0xFFFFFFFF,				//JTAG: TMS
		0xFFFFFFFF,				//JTAG: TCK
		0xFFFFFFFF,				//JTAG: TDO
		0xFFFFFFFF,				//JTAG: TDI
		0xFFFFFFFF,				//I2C: 	SCK
		0xFFFFFFFF, 			//I2C: 	SDA
		0xFFFFFFFF, 			//SPI: 	SCK
		0xFFFFFFFF, 			//SPI: 	MOSI
		0xFFFFFFFF, 			//SPI: 	MISO
		0xFFFFFFFF				//SPI: 	CS
	},
	{
		GPIO_PIN(PORT_D, 1), 	//INTERFACE: IDLE LED
		GPIO_PIN(PORT_G, 0), 	//INTERFACE: ACT LED
		GPIO_PIN(PORT_D, 0), 	//INTERFACE: ERR LED

		0xFFFFFFFF,				//SWD: RST
		0xFFFFFFFF,  			//SWD: SWDIO
		0xFFFFFFFF,				//SWD: SWCLK
		GPIO_PIN(PORT_G, 3),	//JTAG: RST
		GPIO_PIN(PORT_A, 15), 	//JTAG: TRST
		GPIO_PIN(PORT_F, 12),	//JTAG: TMS
		GPIO_PIN(PORT_D, 15),	//JTAG: TCK
		GPIO_PIN(PORT_B, 12),	//JTAG: TDO
		GPIO_PIN(PORT_C, 7),	//JTAG: TDI
		0xFFFFFFFF,				//I2C: 	SCK
		0xFFFFFFFF, 			//I2C: 	SDA
		0xFFFFFFFF, 			//SPI: 	SCK
		0xFFFFFFFF, 			//SPI: 	MOSI
		0xFFFFFFFF, 			//SPI: 	MISO
		0xFFFFFFFF				//SPI: 	CS
	}
};

const gpio_t *get_pin_list(int number)
{
	return pin_list[number];
}

//uint8_t running_status;

//static void adc_init(void);
static void setup_vbus_irq(void);

/* Pins PB[7:5] are used to detect hardware revision.
 * 000 - Original production build.
 * 001 - Mini production build.
 * 010 - Mini V2.0e and later.
 */
int platform_hwversion(void)
{
	static int hwversion = 0;

	return hwversion;
}

void platform_init(void)
{
	/*SCS_DEMCR |= SCS_DEMCR_VC_MON_EN;
#ifdef ENABLE_DEBUG
	void initialise_monitor_handles(void);
	initialise_monitor_handles();
#endif*/
	/* Setup GPIO ports */
	//gpio_init(JTAG_TMS_PIN, GPIO_IN_PU);
	//gpio_init(JTAG_TCK_PIN, GPIO_OUT);
	//gpio_init(JTAG_TDI_PIN, GPIO_OUT);

	/*gpio_init(LED_UART, GPIO_OUT);
	gpio_init(LED_IDLE_RUN, GPIO_OUT);
	gpio_init(LED_ERROR, GPIO_OUT);*/

	/*platform_srst_set_val(false);

	gpio_init(SRST_PIN, GPIO_OUT);*/

	/* Relocate interrupt vector table here */
	//extern int vector_table;
	//SCB_VTOR = (uint32_t)&vector_table;

	//platform_timing_init();
	//cdcacm_init();
	interface_numbers_init();
	cdc_acm_init();
}

void platform_interface_init(gpio_t *pin_list)
{

}

void platform_cdc_start(void)
{
	cdc_acm_start();
}

void platform_srst_set_val(bool assert)
{
	gpio_write(SWD_DIO_PIN, 1);
	gpio_write(SRST_PIN, !assert);
	if (assert) {
		for(int i = 0; i < 10000; i++) __NOP();
	}
}

bool platform_srst_get_val(void)
{
	return (gpio_read(SRST_PIN) == 0);
}

bool platform_target_get_power(void)
{
	return 0;
}

void platform_target_set_power(bool power)
{

}

const char *platform_target_voltage(void)
{
	return "OK";
}

void platform_request_boot(void)
{

}

void platform_delay(uint32_t ms)
{
	xtimer_usleep(ms*1000);
}

uint32_t platform_time_ms(void)
{
	return xtimer_now_usec()/1000;
}
