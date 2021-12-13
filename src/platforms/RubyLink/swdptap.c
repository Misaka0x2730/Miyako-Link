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
	return 0;
}

static void swdptap_turnaround(gpio_t *pin_list, int dir)
{
}

bool swdptap_bit_in(gpio_t *pin_list)
{
    return 0;
}

void swdptap_bit_out(gpio_t *pin_list, bool val)
{
}

uint32_t swdptap_seq_in(int ticks)
{
    return 0;
}

bool swdptap_seq_in_parity(uint32_t *ret, int ticks)
{
	uint8_t parity = 0;

	return parity;
}

void swdptap_seq_out(uint32_t MS, int ticks)
{
}

void swdptap_seq_out_parity(uint32_t MS, int ticks)
{
}
