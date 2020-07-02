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
#include "periph/gpio.h"
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

/* Important pin mappings for STM32 implementation:
 *
 * LED0 = 	PB2	(Yellow LED : Running)
 * LED1 = 	PB10	(Yellow LED : Idle)
 * LED2 = 	PB11	(Red LED    : Error)
 *
 * TPWR = 	RB0 (input) -- analogue on mini design ADC1, ch8
 * nTRST = 	PB1 (output) [blackmagic]
 * PWR_BR = 	PB1 (output) [blackmagic_mini] -- supply power to the target, active low
 * TMS_DIR =    PA1 (output) [blackmagic_mini v2.1] -- choose direction of the TCK pin, input low, output high
 * SRST_OUT = 	PA2 (output)
 * TDI = 	PA3 (output)
 * TMS = 	PA4 (input/output for SWDIO)
 * TCK = 	PA5 (output SWCLK)
 * TDO = 	PA6 (input)
 * nSRST = 	PA7 (input)
 *
 * USB cable pull-up: PA8
 * USB VBUS detect:  PB13 -- New on mini design.
 *                           Enable pull up for compatibility.
 * Force DFU mode button: PB12
 */

#define JTAG_TDI_PIN	(GPIO_PIN(PORT_C, 7))
#define JTAG_TDO_PIN	(GPIO_PIN(PORT_B, 12))
#define JTAG_TMS_PIN	(GPIO_PIN(PORT_F, 12))
#define JTAG_TCK_PIN	(GPIO_PIN(PORT_D, 15))
#define JTAG_TRST_PIN	(GPIO_PIN(PORT_A, 15))

#define SWD_DIO_PIN		(GPIO_PIN(PORT_C, 12))
#define SWD_CLK_PIN		(GPIO_PIN(PORT_D, 2))

#define SRST_PIN		(GPIO_PIN(PORT_G, 3))

#define LED_UART		(GPIO_PIN(PORT_F, 9))
#define LED_IDLE_RUN	(GPIO_PIN(PORT_G, 1))
#define LED_ERROR		(GPIO_PIN(PORT_F, 7))

#define TMS_SET_MODE() do { 			\
	gpio_set(JTAG_TMS_PIN); 			\
	gpio_init(JTAG_TMS_PIN, GPIO_OUT); 	\
} while(0)

#define SWDIO_MODE_FLOAT() do { 		\
	gpio_init(SWD_DIO_PIN, GPIO_IN_PU); \
} while(0)

#define SWDIO_MODE_DRIVE() do { 				\
	gpio_init(SWD_DIO_PIN, GPIO_OUT);			\
	gpio_set(SWD_DIO_PIN); 						\
} while(0)

/*#define TRACE_TIM TIM3
#define TRACE_TIM_CLK_EN() rcc_periph_clock_enable(RCC_TIM3)
#define TRACE_IRQ   NVIC_TIM3_IRQ
#define TRACE_ISR   tim3_isr*/

#ifdef ENABLE_DEBUG
extern bool debug_bmp;
int usbuart_debug_write(const char *buf, size_t len);

#define DEBUG printf
#else
#define DEBUG(...)
#endif

//#define SET_RUN_STATE(running_status, state)	{running_status = (state);}
#define SET_IDLE_STATE(state)	{gpio_write(LED_IDLE_RUN, (state));}
#define SET_ERROR_STATE(state)	{gpio_write(LED_ERROR, (state));}

/* Use newlib provided integer only stdio functions */
#define sscanf siscanf
#define sprintf siprintf
#define snprintf sniprintf
#define vasprintf vasiprintf

const gpio_t *get_pin_list(int number);
void platform_cdc_start(void);

#endif
