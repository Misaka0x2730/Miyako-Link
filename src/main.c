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

/* Provides main entry point.  Initialise subsystems and enter GDB
 * protocol loop.
 */

#include "general.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#include "morse.h"

#define GDB_THREAD_NAME_LENGTH	(20)
#define GDB_STACK_SIZE			(THREAD_STACKSIZE_LARGE*2)
#define MORSE_STACK_SIZE		(THREAD_STACKSIZE_TINY)
#define GDB_THREAD_PRIO			(THREAD_PRIORITY_MAIN-2)
#define MORSE_THREAD_PRIO		(THREAD_PRIORITY_MIN-1)


static char gdb_stack[MAX_GDB_NUMBER][GDB_STACK_SIZE] = {0};
static char morse_stack[INTERFACE_NUMBER][THREAD_STACKSIZE_TINY] = {0};

void *gdb_thread(void *arg)
{
	int iface_number = (int*)arg;

	add_pid_to_list();
	cdc_thread_init();
	set_interface_number(iface_number);

	while (true) {
		volatile struct exception e;
		TRY_CATCH(e, EXCEPTION_ALL) {
			gdb_main();
		}
		if (e.type) {
			gdb_putpacketz("EFF");
			target_list_free();
			morse(get_interface_number(), "TARGET LOST.", 1);
		}
	}
}

void *morse_thread(void *arg)
{
	int number = *(int*)arg;

	while(1) {
		SET_ERROR_STATE(morse_update(number));
		xtimer_usleep(MORSE_PERIOD*1000);
	}
}

void gdb_threads_start(void)
{
	char thread_name[GDB_THREAD_NAME_LENGTH] = {0};
	static int interface_list[INTERFACE_NUMBER] = {0};

	for(int i = 0; i < INTERFACE_NUMBER; ++i) {
		snprintf(thread_name, GDB_THREAD_NAME_LENGTH, "morse_thread_%d", i+1);
		interface_list[i] = i;

		thread_create(morse_stack[i], sizeof(morse_stack[i]),
						MORSE_THREAD_PRIO, 0,
						morse_thread, &interface_list[i], thread_name);
	}

	for(int i = 0; i < MAX_GDB_NUMBER; ++i) {
		snprintf(thread_name, GDB_THREAD_NAME_LENGTH, "gdb_thread_%d", i+1);
		thread_create(gdb_stack[i], sizeof(gdb_stack[i]),
						GDB_THREAD_PRIO-i, 0,
						gdb_thread, &interface_list[i], thread_name);
	}
}

int
main(void)
{
	platform_init();
	gdb_threads_start();
	//__disable_irq();
	platform_cdc_start();
	//__enable_irq();


	while(1);
	return 0;
}

