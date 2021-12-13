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

#include "periph_conf.h"
#include "xtimer.h"
#include "timing.h"
//#include "timing_stm32.h"

//#define PLATFORM_HAS_TRACESWO
//#define PLATFORM_HAS_POWER_SWITCH

#define INTERFACE_NUMBER 			(2)
#define MAX_GDB_NUMBER				(INTERFACE_NUMBER)

#ifdef ENABLE_DEBUG
#define PLATFORM_HAS_DEBUG
#define USBUART_DEBUG
#endif
#define BOARD_IDENT             "Julis Probe"
#define BOARD_IDENT_DFU	        "Julis Probe (Upgrade)"
#define BOARD_IDENT_UPD	        "Julis Probe (DFU Upgrade)"
#define DFU_IDENT               "Julis Probe Firmware Upgrade"
#define UPD_IFACE_STRING        "@Internal Flash   /0x08000000/8*001Kg"

#ifdef ENABLE_DEBUG

#define DEBUG printf
#else
#define DEBUG(...)
#endif

/* Use newlib provided integer only stdio functions */
#define sscanf siscanf
#define sprintf siprintf
#define snprintf sniprintf
#define vasprintf vasiprintf

const gpio_t *get_pin_list(int number);
void platform_cdc_start(void);

#endif
