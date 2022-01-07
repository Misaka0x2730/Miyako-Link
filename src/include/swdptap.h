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

#ifndef __SWDPTAP_H
#define __SWDPTAP_H

#include "adiv5.h"

int swdptap_init(ADIv5_DP_t *dp);

/* Primitive functions */
bool swdptap_bit_in(gpio_t *pin_list);
void swdptap_bit_out(gpio_t *pin_list, bool val);

/* Low level functions, provided in swdptap_generic.c from the primitives
   (indicate NO_OWN_LL in the Makefile.inc or libopencm specific in 
   platforms/common*/
uint32_t swdptap_seq_in(int ticks);
bool swdptap_seq_in_parity(uint32_t *data, int ticks);
void swdptap_seq_out(uint32_t MS, int ticks);
void swdptap_seq_out_parity(uint32_t MS, int ticks);

#endif

