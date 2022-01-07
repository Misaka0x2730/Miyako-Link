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
#include "FreeRTOS.h"
#include "task.h"

const gpio_t *get_pin_list(int number)
{
    return 0;
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

}

void platform_interface_init(gpio_t *pin_list)
{

}

void platform_cdc_start(void)
{
}

bool platform_srst_get_val(void)
{
	return (0);
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
    vTaskDelay(ms);
}

uint32_t platform_time_ms(void)
{
    const uint64_t current_time = System_GetUptime();
	return (uint32_t)(current_time / 1000);
}

uint32_t platform_max_frequency_get(void)
{
    return 4000000;
}

void platform_max_frequency_set(uint32_t frequency)
{
    (void) frequency;
}