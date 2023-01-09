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

#include "general.h"
//#include "cdcacm.h"
//#include "usbuart.h"
#include "morse.h"
#include "platform.h"
#include "gdb_if.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device_settings.h"
#include "system_pins.h"
#include "hardware/structs/systick.h"
#include "hardware/structs/scb.h"

#include "hardware/uart.h"
#include "platform_target_interface_non_iso.h"

static float platform_calculate_pio_clkdiv(uint32_t frequency);

/* Pins PB[7:5] are used to detect hardware revision.
 * 000 - Original production build.
 * 001 - Mini production build.
 * 010 - Mini V2.0e and later.
 */
int platform_hwversion(void)
{
	return 0;
}

void platform_init(void)
{

}

const char *platform_target_voltage(void)
{
	return "not supported";
}

void platform_delay(uint32_t ms)
{
    vTaskDelay(ms);
}

uint32_t platform_time_ms(void)
{
    const uint64_t current_time = System_GetUptime();
	return (uint32_t)(current_time / 1000);
}

static float platform_calculate_pio_clkdiv(uint32_t frequency)
{
    if ((frequency < PIO_MIN_FREQUENCY) || (frequency > PIO_MAX_FREQUENCY))
    {
        return -1;
        //return ((float)PIO_SYSCLK / PIO_DEFAULT_FREQUENCY) / 2;
    }

    return ((float)PIO_SYSCLK / (float)frequency) / 2;
}

static bool platform_max_frequency_set(struct target_controller *tc, uint32_t frequency)
{
    if ((frequency < tc->target_interface_param.min_frequency) || (frequency > tc->target_interface_param.max_frequency))
    {
        return false;
    }

    float clkdiv = platform_calculate_pio_clkdiv(frequency);
    if (clkdiv < 0)
    {
        return false;
    }

    tc->target_interface_param.clkdiv = clkdiv;
    tc->target_interface_param.current_frequency = frequency;
    tc->target_interface_param.update_clkdiv = true;

    return true;
}

void platform_target_interface_max_frequency_set(struct target_controller *tc, uint32_t frequency)
{
    if (platform_max_frequency_set(tc, frequency))
    {
        /*device_settings_set_value(tc->target_interface_param.frequency_setting_id, (uint8_t *) &frequency,
                                  sizeof(frequency));*/
    }
}

uint32_t platform_target_interface_max_frequency_get(struct target_controller *tc)
{
    return tc->target_interface_param.current_frequency;
}

uint32_t platform_target_voltage_sense(void)
{
    return 0;
}

void platform_target_interface_non_iso_init(struct target_controller *tc)
{
    if (tc != NULL)
    {
        tc->target_interface_param.current_program = PIO_PROGRAM_NOT_SET;
        tc->target_interface_param.min_frequency = TARGET_INTERFACE_1_MIN_FREQUENCY;
        tc->target_interface_param.max_frequency = TARGET_INTERFACE_1_MAX_FREQUENCY;
        tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED;
        //tc->target_interface_param.frequency_setting_id = DEVICE_SETTINGS_TARGET_INTERFACE_CONFIG_1_FREQ;

        tc->platform_get_voltage = platform_target_voltage;
        tc->platform_srst_set_val = platform_target_interface_non_iso_srst_set_val;
        tc->platform_srst_get_val = platform_target_interface_non_iso_srst_get_val;
        tc->platform_target_set_power = platform_target_interface_non_iso_set_power;
        tc->platform_target_get_power = platform_target_interface_non_iso_get_power;
        tc->platform_target_voltage_sense = platform_target_voltage_sense;
        tc->platform_target_set_idle_state = platform_target_interface_non_iso_set_idle_state;
        tc->platform_target_set_error_state = platform_target_interface_non_iso_set_error_state;
        tc->swdptap_seq_out_parity = platform_target_interface_non_iso_seq_out_parity;
        tc->swdptap_seq_out = platform_target_interface_non_iso_seq_out;
        tc->swdptap_seq_in = platform_target_interface_non_iso_seq_in;
        tc->swdptap_seq_in_parity = platform_target_interface_non_iso_seq_in_parity;
        tc->swdptap_low_write = platform_target_interface_non_iso_swd_low_write;
        tc->swdptap_low_access = platform_target_interface_non_iso_swd_low_access;
        tc->jtagtap_next = platform_target_interface_non_iso_jtag_next;
        tc->jtagtap_tms_seq = platform_target_interface_non_iso_jtag_tms_seq;
        tc->jtagtap_tdi_tdo_seq = platform_target_interface_non_iso_jtag_tdi_tdo_seq;
        tc->jtagtap_tdi_seq = platform_target_interface_non_iso_jtag_tdi_seq;
        tc->platform_max_frequency_set = platform_target_interface_max_frequency_set;
        tc->platform_max_frequency_get = platform_target_interface_max_frequency_get;
        tc->jtag_proc.jtagtap_init = jtagtap_init;

        platform_max_frequency_set(tc, TARGET_INTERFACE_1_DEFAULT_FREQUENCY);
    }
}

U32 SEGGER_SYSVIEW_X_GetTimestamp(void)
{
    /*U32 TickCount;
    U32 Cycles;
    U32 CyclesPerTick;
    //
    // Get the cycles of the current system tick.
    // SysTick is down-counting, subtract the current value from the number of cycles per tick.
    //
    CyclesPerTick = systick_hw->rvr + 1;
    Cycles = (CyclesPerTick - systick_hw->cvr);
    //
    // Get the system tick count.
    //
    TickCount = SEGGER_SYSVIEW_TickCnt;
    //
    // If a SysTick interrupt is pending, re-read timer and adjust result
    //
    if ((scb_hw->icsr & 0x04000000) != 0)
    {
        Cycles = (CyclesPerTick - systick_hw->cvr);
        TickCount++;
    }
    Cycles += TickCount * CyclesPerTick;*/

    return time_us_32();

    //return Cycles;
}