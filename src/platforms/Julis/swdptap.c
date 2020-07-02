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

/* This file implements the SW-DP interface. */

#include "general.h"
#include "swdptap.h"

enum {
	SWDIO_STATUS_FLOAT = 0,
	SWDIO_STATUS_DRIVE
};

int swdptap_init(void)
{
	gpio_t *pin_list = get_pin_list(get_interface_number());

	gpio_init(pin_list[PIN_SWD_CLK], GPIO_OUT);
	gpio_init(pin_list[PIN_SWD_DIO], GPIO_OUT);

	gpio_init(pin_list[PIN_LED_MISC_1], GPIO_OUT);
	gpio_init(pin_list[PIN_LED_IDLE], GPIO_OUT);
	gpio_init(pin_list[PIN_LED_ERR], GPIO_OUT);

	platform_srst_set_val(false);

	gpio_init(pin_list[PIN_SWD_RST], GPIO_OUT);

	return 0;
}

static void swdptap_turnaround(gpio_t *pin_list, int dir)
{
	static int olddir = SWDIO_STATUS_FLOAT;

	/* Don't turnaround if direction not changing */
	if(dir == olddir) return;
	olddir = dir;

#ifdef DEBUG_SWD_BITS
	DEBUG("%s", dir ? "\n-> ":"\n<- ");
#endif

	if(dir == SWDIO_STATUS_FLOAT) {
		gpio_init(pin_list[PIN_SWD_DIO], GPIO_IN_PU);
	}
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_clear(pin_list[PIN_SWD_CLK]);
	if(dir == SWDIO_STATUS_DRIVE) {
		gpio_init(pin_list[PIN_SWD_DIO], GPIO_OUT);
		gpio_set(pin_list[PIN_SWD_DIO]);
	}
}

bool swdptap_bit_in(gpio_t *pin_list)
{
	uint16_t ret;

	swdptap_turnaround(pin_list, SWDIO_STATUS_FLOAT);

	ret = gpio_read(pin_list[PIN_SWD_DIO]);
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_clear(pin_list[PIN_SWD_CLK]);

#ifdef DEBUG_SWD_BITS
	DEBUG("%d", ret?1:0);
#endif

	return ret != 0;
}

void swdptap_bit_out(gpio_t *pin_list, bool val)
{
#ifdef DEBUG_SWD_BITS
	DEBUG("%d", val);
#endif

	swdptap_turnaround(pin_list, SWDIO_STATUS_DRIVE);

	gpio_write(pin_list[PIN_SWD_DIO], val);
	gpio_clear(pin_list[PIN_SWD_CLK]);
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_set(pin_list[PIN_SWD_CLK]);
	gpio_clear(pin_list[PIN_SWD_CLK]);
}
uint32_t swdptap_seq_in(int ticks)
{
	uint32_t index = 1;
	uint32_t ret = 0;
	gpio_t *pin_list = get_pin_list(get_interface_number());

	while (ticks--) {
		if (swdptap_bit_in(pin_list))
			ret |= index;
		index <<= 1;
	}

	return ret;
}

bool swdptap_seq_in_parity(uint32_t *ret, int ticks)
{
	uint32_t index = 1;
	uint8_t parity = 0;
	gpio_t *pin_list = get_pin_list(get_interface_number());

	*ret = 0;

	while (ticks--) {
		if (swdptap_bit_in(pin_list)) {
			*ret |= index;
			parity ^= 1;
		}
		index <<= 1;
	}
	if (swdptap_bit_in(pin_list))
		parity ^= 1;

	return parity;
}

void swdptap_seq_out(uint32_t MS, int ticks)
{
	gpio_t *pin_list = get_pin_list(get_interface_number());

	while (ticks--) {
		swdptap_bit_out(pin_list, MS & 1);
		MS >>= 1;
	}
}

void swdptap_seq_out_parity(uint32_t MS, int ticks)
{
	uint8_t parity = 0;
	gpio_t *pin_list = get_pin_list(get_interface_number());

	while (ticks--) {
		swdptap_bit_out(pin_list, MS & 1);
		parity ^= MS;
		MS >>= 1;
	}
	swdptap_bit_out(pin_list, parity & 1);
}
