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
#ifndef __PLATFORM_H
#define __PLATFORM_H

#include "include/periph_conf.h"
#include "timing.h"
#include "hardware/gpio.h"
//#include "timing_stm32.h"

//TODO: change to thread-safe variant
extern uint8_t running_status;

//#define PLATFORM_HAS_TRACESWO
//#define PLATFORM_HAS_POWER_SWITCH

#define PLATFORM_IDENT      "(RubyLink) "

#define INTERFACE_NUMBER 			(2)
#define MAX_GDB_NUMBER				(INTERFACE_NUMBER)

#ifdef ENABLE_DEBUG
#define PLATFORM_HAS_DEBUG
#define USBUART_DEBUG
#endif
/*#define BOARD_IDENT             "Julis Probe"
#define BOARD_IDENT_DFU	        "Julis Probe (Upgrade)"
#define BOARD_IDENT_UPD	        "Julis Probe (DFU Upgrade)"
#define DFU_IDENT               "Julis Probe Firmware Upgrade"
#define UPD_IFACE_STRING        "@Internal Flash   /0x08000000/8*001Kg"*/

#define SET_RUN_STATE(state)	{tc->running_status = (state);}
#define SET_IDLE_STATE(state)
#define SET_ERROR_STATE(state)

#ifdef ENABLE_DEBUG

#define DEBUG printf
#else
#define DEBUG(...)
#endif

#define TDI_PIN		19
#define TMS_PIN		17
#define TCK_PIN		18
#define TDO_PIN		14

#define SWDIO_PIN	TMS_PIN
#define SWCLK_PIN	TCK_PIN

#define TMS_DIR_PIN	    16
#define SWDIO_DIR_PIN	TMS_DIR_PIN

#define TMS_SET_MODE() do { \
	gpio_set_dir(TMS_DIR_PIN, GPIO_OUT); \
    gpio_put(TMS_DIR_PIN, 1);   \
} while(0)

#define SWDIO_MODE_FLOAT() do { \
    gpio_put(TMS_DIR_PIN, 0);                             \
    gpio_pull_up(SWDIO_PIN);          \
	gpio_set_dir(SWDIO_PIN, GPIO_IN); \
} while(0)

#define SWDIO_MODE_DRIVE() do { \
    gpio_put(TMS_DIR_PIN, 1);          \
	gpio_set_dir(SWDIO_PIN, GPIO_OUT);    \
    gpio_put(SWDIO_PIN, 1);                              \
} while(0)

const gpio_t *get_pin_list(int number);
void platform_cdc_start(void);
uint32_t platform_max_frequency_get(void);

#endif
