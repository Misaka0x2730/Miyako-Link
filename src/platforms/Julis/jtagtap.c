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

/* This file implements the low-level JTAG TAP interface.  */

#include <stdio.h>

#include "general.h"
#include "jtagtap.h"
#include "gdb_packet.h"

int jtagtap_init(void)
{
	gpio_t *pin_list = get_pin_list(get_interface_number());
	gpio_set(pin_list[PIN_JTAG_TMS]);
	gpio_init(pin_list[PIN_JTAG_TMS], GPIO_OUT);

	/* Go to JTAG mode for SWJ-DP */
	for(int i = 0; i <= 50; i++) jtagtap_next(1, 0); /* Reset SW-DP */
	jtagtap_tms_seq(0xE73C, 16);		/* SWD to JTAG sequence */
	jtagtap_soft_reset();

	return 0;
}

void jtagtap_reset(void)
{
#ifdef JTAG_TRST_PIN
	if (platform_hwversion() == 0) {
		volatile int i;
		gpio_clear(JTAG_TRST_PIN);
		for(i = 0; i < 10000; i++) __NOP();
		gpio_set(JTAG_TRST_PIN);
	}
#endif
	jtagtap_soft_reset();
}

inline uint8_t jtagtap_next(uint8_t dTMS, uint8_t dTDI)
{
	uint16_t ret;

	gpio_write(JTAG_TMS_PIN, dTMS);
	gpio_write(JTAG_TDI_PIN, dTDI);
	gpio_set(JTAG_TCK_PIN);
	ret = gpio_read(JTAG_TDO_PIN);
	gpio_clear(JTAG_TCK_PIN);

	//DEBUG("jtagtap_next(TMS = %d, TDI = %d) = %d\n", dTMS, dTDI, ret);

	return ret != 0;
}

void jtagtap_tms_seq(uint32_t MS, int ticks)
{
	gpio_write(JTAG_TDI_PIN, 1);
	int data = MS & 1;
	while(ticks) {
		gpio_write(JTAG_TMS_PIN, data);
		gpio_set(JTAG_TCK_PIN);
		MS >>= 1;
		data = MS & 1;
		ticks--;
		gpio_clear(JTAG_TCK_PIN);
	}
}

void
jtagtap_tdi_tdo_seq(uint8_t *DO, const uint8_t final_tms, const uint8_t *DI, int ticks)
{
	uint8_t index = 1;
	gpio_write(JTAG_TMS_PIN, 0);
	uint8_t res = 0;
	while(ticks > 1) {
		gpio_write(JTAG_TDI_PIN, *DI & index);
		gpio_set(JTAG_TCK_PIN);
		if (gpio_read(JTAG_TDO_PIN)) {
			res |= index;
		}
		if(!(index <<= 1)) {
			*DO = res;
			res = 0;
			index = 1;
			DI++; DO++;
		}
		ticks--;
		gpio_clear(JTAG_TCK_PIN);
	}
	gpio_write(JTAG_TMS_PIN, final_tms);
	gpio_write(JTAG_TDI_PIN, *DI & index);
	gpio_set(JTAG_TCK_PIN);
	if (gpio_read(JTAG_TDO_PIN)) {
		res |= index;
	}
	*DO = res;
	gpio_clear(JTAG_TCK_PIN);
}

void
jtagtap_tdi_seq(const uint8_t final_tms, const uint8_t *DI, int ticks)
{
	uint8_t index = 1;
	while(ticks--) {
		gpio_write(JTAG_TMS_PIN, ticks? 0 : final_tms);
		gpio_write(JTAG_TDI_PIN, *DI & index);
		gpio_set(JTAG_TCK_PIN);
		if(!(index <<= 1)) {
			index = 1;
			DI++;
		}
		gpio_clear(JTAG_TCK_PIN);
	}
}
