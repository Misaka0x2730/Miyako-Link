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
#include "adiv5.h"
#include "gdb_packet.h"

void jtagtap_reset(struct target_controller *tc);

void jtagtap_init(struct target_controller *tc)
{
    //struct target_controller_s *tc = dp->tc;
    struct jtag_proc_s *jtag_proc = &(tc->jtag_proc);

    tc->jtag_proc.jtagtap_reset = jtagtap_reset;
    tc->jtag_proc.jtagtap_next = tc->jtagtap_next;
    tc->jtag_proc.jtagtap_tms_seq = tc->jtagtap_tms_seq;
    tc->jtag_proc.jtagtap_tdi_tdo_seq = tc->jtagtap_tdi_tdo_seq;
    tc->jtag_proc.jtagtap_tdi_seq = tc->jtagtap_tdi_seq;

#if (PIO_JTAG == 0)
    gpio_init(PIN_TARGET1_TCK);
    gpio_set_dir(PIN_TARGET1_TCK, GPIO_OUT);
    gpio_put(PIN_TARGET1_TCK, 0);

    gpio_init(PIN_TARGET1_TDI);
    gpio_set_dir(PIN_TARGET1_TDI, GPIO_OUT);
    gpio_put(PIN_TARGET1_TDI, 0);

    gpio_init(PIN_TARGET1_TDO);
    gpio_set_dir(PIN_TARGET1_TDO, GPIO_IN);

    gpio_init(PIN_TARGET1_TMS_DIR_PIN);
    gpio_set_dir(PIN_TARGET1_TMS_DIR_PIN, GPIO_OUT);
    gpio_put(PIN_TARGET1_TMS_DIR_PIN, 1);

    gpio_init(PIN_TARGET1_TMS);
    gpio_set_dir(PIN_TARGET1_TMS, GPIO_OUT);
    gpio_put(PIN_TARGET1_TMS, 0);
#endif
    /* Go to JTAG mode for SWJ-DP */
    for(int i = 0; i <= 50; i++)
    {
        tc->jtag_proc.jtagtap_next(tc, 1, 0); /* Reset SW-DP */
    }
    tc->jtag_proc.jtagtap_tms_seq(tc, 0xE73C, 16);		/* SWD to JTAG sequence */
    jtagtap_soft_reset();
}

void jtagtap_reset(struct target_controller *tc)
{
    struct jtag_proc_s *jtag_proc = &(tc->jtag_proc);

    jtagtap_soft_reset();
}

