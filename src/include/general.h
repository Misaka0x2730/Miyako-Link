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

#ifndef __GENERAL_H
#define __GENERAL_H

#define _GNU_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <sys/types.h>

#include "platform.h"
#include "platform_support.h"

#include "pico/mutex.h"
#include "FreeRTOS.h"
#include "task.h"
#include "logging.h"

typedef TaskHandle_t kernel_pid_t;

extern uint32_t delay_cnt;

enum BMP_DEBUG {
    BMP_DEBUG_NONE   =  0,
    BMP_DEBUG_INFO   =  1,
    BMP_DEBUG_GDB    =  2,
    BMP_DEBUG_TARGET =  4,
    BMP_DEBUG_PROBE =  8,
    BMP_DEBUG_WIRE   = 0x10,
    BMP_DEBUG_MAX    = 0x20,
    BMP_DEBUG_STDOUT = 0x8000,
};

#define FREQ_FIXED 0xffffffff

#define DEBUG_WARN LogWarn
#define DEBUG_INFO LogInfo
#define DEBUG_GDB(...) do {} while(0)
#define DEBUG_GDB_WIRE(...) do {} while(0)
# define DEBUG_TARGET(...) do {} while(0)

#define ALIGN(x, n) (((x) + (n) - 1) & ~((n) - 1))
#undef MIN
#define MIN(x, y)  (((x) < (y)) ? (x) : (y))
#undef MAX
#define MAX(x, y)  (((x) > (y)) ? (x) : (y))

#if !defined(SYSTICKHZ)
# define SYSTICKHZ 100
#endif
#define SYSTICKMS (1000 / SYSTICKHZ)
#define MORSECNT ((SYSTICKHZ / 10) - 1)

#define MAX_INTERFACE_PINS		(32)

typedef enum {
	PIN_LED_IDLE = 0,
	PIN_LED_ERR,
	PIN_LED_MISC_1,

	PIN_SWD_RST,
	PIN_SWD_DIO,
	PIN_SWD_CLK,
	PIN_JTAG_RST,
	PIN_JTAG_TRST,
	PIN_JTAG_TMS,
	PIN_JTAG_TCK,
	PIN_JTAG_TDO,
	PIN_JTAG_TDI,

	//PIN_I2C_SCK,
	//PIN_I2C_SDA,

	PIN_SPI_SCK,
	PIN_SPI_MOSI,
	PIN_SPI_MISO,
	PIN_SPI_CS
}interface_pins;

void add_pid_to_list(void);
int get_thread_number(void);

#define DEF_THREAD_RESOURCE(type, name)					\
static type name##_list[MAX_GDB_NUMBER] = {0};			\
static type get_##name(void)							\
{														\
	return name##_list[get_thread_number()];			\
}														\
static type *get_ptr_##name(void)						\
{														\
	return &name##_list[get_thread_number()];			\
}														\
static void set_##name(type t)							\
{														\
	name##_list[get_thread_number()] = t;				\
}

#define DEF_THREAD_RESOURCE_NON_STATIC(type, name)		\
static type name##_list[MAX_GDB_NUMBER] = {0};			\
type get_##name(void)									\
{														\
	return name##_list[get_thread_number()];			\
}														\
void set_##name(type t)									\
{														\
	name##_list[get_thread_number()] = t;				\
}

#define DEF_THREAD_RESOURCE_ARRAY(type, name, size)		\
static type name##_list[MAX_GDB_NUMBER][size] = {0};	\
static type *get_##name(void)							\
{														\
	return name##_list[get_thread_number()];			\
}

int get_interface_number(void);
bool set_interface_number(int number);
void interface_numbers_init(void);
bool free_interface(int number);

void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif

