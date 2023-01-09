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
#include "hardware/pio.h"

#define PLATFORM_PIN_INVALID    (0xFF)

typedef enum
{
    PLATFORM_TARGET_INTERFACE_JTAG  = 0,
    PLATFORM_TARGET_INTERFACE_SWD
} platform_target_interface_mode_t;

#define PLATFORM_TARGET_INTERFACE_NUM     (2)

#define PLATFORM_USB_UART_INTERFACE_NUM   (2)

//TODO: change to thread-safe variant

//#define PLATFORM_HAS_TRACESWO
//#define PLATFORM_HAS_POWER_SWITCH

#define PLATFORM_IDENT      "(RubyLink) "

#define INTERFACE_NUMBER 			(2)
#define MAX_GDB_NUMBER				(INTERFACE_NUMBER)

#define PLATFORM_HAS_POWER_SWITCH

#ifdef ENABLE_DEBUG
#define PLATFORM_HAS_DEBUG

#define USBUART_DEBUG
#endif
/*#define BOARD_IDENT           "Julis Probe"
#define BOARD_IDENT_DFU	        "Julis Probe (Upgrade)"
#define BOARD_IDENT_UPD	        "Julis Probe (DFU Upgrade)"
#define DFU_IDENT               "Julis Probe Firmware Upgrade"
#define UPD_IFACE_STRING        "@Internal Flash   /0x08000000/8*001Kg"*/

#define SET_RUN_STATE(state)	{tc->running_status = (state);}
#define SET_IDLE_STATE(state)   {tc->platform_target_set_idle_state(state);}
#define SET_ERROR_STATE(state)  {tc->platform_target_set_error_state(state);}

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


#define USE_PIO     (1)
#define PIO_JTAG    (1)

#define PIO_MIN_FREQUENCY                       (1000)
#define PIO_MAX_FREQUENCY                       (40000000)
#define PIO_DEFAULT_FREQUENCY                   (1000000)

#define TARGET_INTERFACE_DEFAULT_FREQUENCY      (PIO_DEFAULT_FREQUENCY)

#define TARGET_INTERFACE_1_MIN_FREQUENCY        (PIO_MIN_FREQUENCY)
#define TARGET_INTERFACE_1_MAX_FREQUENCY        (20000000)
#define TARGET_INTERFACE_1_DEFAULT_FREQUENCY    (TARGET_INTERFACE_DEFAULT_FREQUENCY)

#define TARGET_INTERFACE_2_MIN_FREQUENCY        (PIO_MIN_FREQUENCY)
#define TARGET_INTERFACE_2_MAX_FREQUENCY        (10000000)
#define TARGET_INTERFACE_2_DEFAULT_FREQUENCY    (TARGET_INTERFACE_DEFAULT_FREQUENCY)

#define PIO_SYSCLK              (125000000)

typedef enum
{
    PIO_PROGRAM_NOT_SET = 0,
    PIO_PROGRAM_SWD_DP_LOW_ACCESS_READ,
    PIO_PROGRAM_SWD_DP_LOW_ACCESS_WRITE,
    PIO_PROGRAM_SWD_DP_LOW_SEQ_OUT,
    PIO_PROGRAM_SWD_DP_LOW_SEQ_IN,
    PIO_PROGRAM_SWD_DP_LOW_WRITE,
    PIO_PROGRAM_JTAG_DP_TDI_TDO_SEQ,
    PIO_PROGRAM_JTAG_DP_TDI_SEQ,
    PIO_PROGRAM_JTAG_DP_TMS_SEQ,
    PIO_PROGRAM_JTAG_DP_NEXT
} target_interface_pio_program_t;

#define TARGET_INTERFACE_PROGRAM_IS_SWD(program)    (((program) >= PIO_PROGRAM_SWD_DP_LOW_ACCESS_READ) && ((program) <= PIO_PROGRAM_SWD_DP_LOW_WRITE))
#define TARGET_INTERFACE_PROGRAM_IS_JTAG(program)   (((program) >= PIO_PROGRAM_JTAG_DP_TDI_TDO_SEQ) && ((program) <= PIO_PROGRAM_JTAG_DP_NEXT))

typedef enum {
    TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED = 0,
    TARGET_INTERFACE_TMS_DIR_IN,
    TARGET_INTERFACE_TMS_DIR_OUT
} target_interface_tms_dir_t;

typedef struct
{
    uint32_t target_interface_number;
    target_interface_pio_program_t current_program;
    PIO pio_number;
    uint32_t sm_number;
    uint32_t min_frequency;
    uint32_t max_frequency;
    uint32_t default_frequency;
    uint32_t current_frequency;
    uint32_t frequency_setting_id;
    float clkdiv;
    bool update_clkdiv;
    target_interface_tms_dir_t tms_direction;
} target_interface_param_t;

struct target_controller;

void platform_target_interface_non_iso_init(struct target_controller *tc);
const gpio_t *get_pin_list(int number);
void platform_cdc_start(void);
uint32_t platform_min_frequency_get(struct target_controller *tc);

#endif
