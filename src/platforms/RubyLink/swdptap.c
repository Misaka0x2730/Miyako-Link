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
#include "hardware/pio.h"

int swdptap_init(ADIv5_DP_t *dp)
{
    dp->seq_in  = dp->tc->swdptap_seq_in;
    dp->seq_in_parity  = dp->tc->swdptap_seq_in_parity;
    dp->seq_out = dp->tc->swdptap_seq_out;
    dp->seq_out_parity  = dp->tc->swdptap_seq_out_parity;
	return 0;
}