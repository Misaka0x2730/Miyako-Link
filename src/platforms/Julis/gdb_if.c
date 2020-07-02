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
//#include "cdcacm.h"
#include "gdb_if.h"

#include "usb/usbus.h"
#include "usb/usbus/cdc/acm.h"

#include "isrpipe.h"
#include "isrpipe/read_timeout.h"
#include "mutex.h"

#define CDC_BUF_SIZE	(256)

//static usbus_cdcacm_device_t cdcacm;
//static uint8_t _cdc_rx_buf_mem[256];

static usbus_t usbus;
static mutex_t usbus_mutex;
static char _stack[USBUS_STACKSIZE];

/*DEF_THREAD_RESOURCE_ARRAY(uint8_t, cdc_tx_buf, CDC_BUF_SIZE);
DEF_THREAD_RESOURCE_ARRAY(uint8_t, cdc_rx_buf, CDC_BUF_SIZE);
DEF_THREAD_RESOURCE(isrpipe_t, cdc_isrpipe);
DEF_THREAD_RESOURCE(usbus_cdcacm_device_t, cdc_device);*/

typedef struct {
	usbus_cdcacm_device_t cdc_acm;
	isrpipe_t isrpipe;
	uint8_t rx_buf[CDC_BUF_SIZE];
	uint8_t tx_buf[CDC_BUF_SIZE];
}gdb_cdc_acm_t;

DEF_THREAD_RESOURCE(gdb_cdc_acm_t, gdb_cdc);

void gdb_if_putchar(unsigned char c, int flush)
{
	gdb_cdc_acm_t *gdb_cdc_acm = get_ptr_gdb_cdc();
	size_t n = usbus_cdc_acm_submit(&(gdb_cdc_acm->cdc_acm), &c, 1);
	if(flush || (n == 0)) {
		//cdc_acm = get_ptr_gdb_cdc()->cdc_acm;
		//usbus_cdc_acm_submit(&cdc_acm, buffer_in, count_in);
		usbus_cdc_acm_flush(&(gdb_cdc_acm->cdc_acm));
		if(n == 0) {
			usbus_cdc_acm_submit(&(gdb_cdc_acm->cdc_acm), &c, 1);
		}
		//stdio_write(buffer_in, count_in);
		//count_in = 0;
	}
}

unsigned char gdb_if_getchar(void)
{
	uint8_t data = 0;
	isrpipe_read(&(get_ptr_gdb_cdc()->isrpipe), &data, 1);
	return data;
}

unsigned char gdb_if_getchar_to(int timeout)
{
	platform_timeout t;
	platform_timeout_set(&t, timeout);
	uint8_t data = 0;
	int res = -1;
	gdb_cdc_acm_t *gdb_cdc_acm = get_ptr_gdb_cdc();

	res = isrpipe_read_timeout(&(gdb_cdc_acm->isrpipe), &data, 1, timeout);

	if(res < 0)
		return -1;
	else
		return data;
}

void cdc_acm_init(void)
{
	/* Get driver context */
	usbdev_t *usbdev = usbdev_get_ctx(0);
	assert(usbdev);

	/* Initialize basic usbus struct, don't start the thread yet */
	usbus_init(&usbus, usbdev);
	for(size_t i = 0; i < MAX_GDB_NUMBER; ++i) {
		isrpipe_init(&(gdb_cdc_list[i].isrpipe), gdb_cdc_list[i].rx_buf, CDC_BUF_SIZE);
	}
}

void cdc_acm_start(void)
{
	usbus_create(_stack, USBUS_STACKSIZE, USBUS_PRIO, USBUS_TNAME, &usbus);
}

static void cdc_rx_cb(usbus_cdcacm_device_t *cdcacm,
                             uint8_t *data, size_t len)
{
    (void)cdcacm;
    gdb_cdc_acm_t *gdb_cdc_acm = container_of(cdcacm, gdb_cdc_acm_t, cdc_acm);

    for (size_t i = 0; i < len; i++) {
        isrpipe_write_one(&(gdb_cdc_acm->isrpipe), data[i]);
    }
}

void cdc_thread_init(void)
{
	mutex_lock(&usbus_mutex);
    usbus_cdc_acm_init(&usbus, &(get_ptr_gdb_cdc()->cdc_acm), cdc_rx_cb, NULL,
    		get_ptr_gdb_cdc()->tx_buf, CDC_BUF_SIZE);
    mutex_unlock(&usbus_mutex);
}

