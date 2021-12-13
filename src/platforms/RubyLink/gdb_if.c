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

/* This file implements a transparent channel over which the GDB Remote
 * Serial Debugging protocol is implemented.  This implementation for STM32
 * uses the USB CDC-ACM device bulk endpoints to implement the channel.
 */
#include "general.h"
#include "gdb_if.h"

#include "isrpipe.h"
#include "isrpipe/read_timeout.h"
#include "mutex.h"

static mutex_t usbus_mutex;

typedef struct {
	isrpipe_t isrpipe;
}gdb_cdc_acm_t;

void gdb_if_putchar(unsigned char c, int flush)
{
}

unsigned char gdb_if_getchar(void)
{
	uint8_t data = 0;
	return data;
}

unsigned char gdb_if_getchar_to(int timeout)
{
	platform_timeout t;
	platform_timeout_set(&t, timeout);
	uint8_t data = 0;
	int res = -1;

	if(res < 0)
		return -1;
	else
		return data;
}

void cdc_acm_init(void)
{
}

void cdc_acm_start(void)
{
}

void cdc_thread_init(void)
{
	mutex_lock(&usbus_mutex);
    mutex_unlock(&usbus_mutex);
}

