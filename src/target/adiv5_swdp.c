/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2020- 2021 Uwe Bonnes (bon@elektron.ikp.physik.tu-darmstadt.de)
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

/* This file implements the SW-DP specific functions of the
 * ARM Debug Interface v5 Architecure Specification, ARM doc IHI0031A.
 */

#include "general.h"
#include "exception.h"
#include "adiv5.h"
#include "target.h"
#include "target_internal.h"
#include "hardware/pio.h"
#include "swd_dp_low_access_write.pio.h"
#include "swd_dp_low_access_read.pio.h"
#include "swd_dp_low_write.pio.h"

unsigned int make_packet_request(uint8_t RnW, uint16_t addr)
{
	bool APnDP = addr & ADIV5_APnDP;
	addr &= 0xff;
	unsigned int request = 0x81; /* Park and Startbit */
	if(APnDP) request ^= 0x22;
	if(RnW)   request ^= 0x24;

	addr &= 0xC;
	request |= (addr << 1) & 0x18;
	if((addr == 4) || (addr == 8))
		request ^= 0x20;
	return request;
}

/* Provide bare DP access functions without timeout and exception */

static void dp_line_reset(ADIv5_DP_t *dp)
{
	dp->seq_out(dp->tc, 0xFFFFFFFF, 32);
	dp->seq_out(dp->tc, 0x0FFFFFFF, 32);
}

bool firmware_dp_low_write(ADIv5_DP_t *dp, uint16_t addr, const uint32_t data)
{
/*#if (USE_PIO != 0)
    unsigned int request = make_packet_request(ADIV5_LOW_WRITE, addr & 0xf);
    request |= (1 << 9);
    uint32_t res = 0;
    uint32_t ack;
    platform_timeout timeout;

    pio_clear_instruction_memory(pio0);
    int prog_offset = pio_add_program(pio0, &swd_dp_low_write_program);
    pio_sm_config prog_config = swd_dp_low_write_program_get_default_config(prog_offset);

    sm_config_set_out_pins(&prog_config, TARGET1_TMS, 1);
    sm_config_set_set_pins(&prog_config, TARGET1_TMS_DIR_PIN, 1);
    sm_config_set_in_pins(&prog_config, TARGET1_TMS);
    sm_config_set_sideset_pins(&prog_config, TARGET1_TCK);

    sm_config_set_out_shift(&prog_config, true, false, 32);
    sm_config_set_in_shift(&prog_config, true, false, 32);
    sm_config_set_clkdiv(&prog_config, dp->tc->target_interface_param.clkdiv);

    const uint32_t swd_all_pins = (1 << TARGET1_TMS) | (1 << TARGET1_TMS_DIR_PIN) | (1 << TARGET1_TCK);
    pio_sm_set_pins_with_mask(pio0, 0, (1 << TARGET1_TMS_DIR_PIN), swd_all_pins);
    pio_sm_set_pindirs_with_mask(pio0, 0, swd_all_pins, swd_all_pins);

    pio_gpio_init(pio0, TARGET1_TMS_DIR_PIN);
    pio_gpio_init(pio0, TARGET1_TCK);
    pio_gpio_init(pio0, TARGET1_TMS);

    pio_sm_init(pio0, 0, prog_offset, &prog_config);
    pio_sm_set_enabled(pio0, 0, true);

    int parity = __builtin_popcount(data);
    pio_sm_put_blocking(pio0, 0, 0x20000000);
    pio_sm_put_blocking(pio0, 0, request);
    pio_sm_put_blocking(pio0, 0, data);
    pio_sm_put_blocking(pio0, 0, parity & 1);

    res = (pio_sm_get_blocking(pio0, 0) >> 29);
    pio_sm_set_enabled(pio0, 0, false);
#else*/
	unsigned int request = make_packet_request(ADIV5_LOW_WRITE, addr & 0xf);
	dp->seq_out(dp->tc, request, 8);
	uint32_t res = dp->seq_in(dp->tc, 3);
	dp->seq_out_parity(dp->tc, data, 32);
//#endif
	return (res != SWDP_ACK_OK);
}

/* Try first the dormant to SWD procedure.
 * If target id given, scan DPs 0 .. 15 on that device and return.
 * Otherwise
 */
int adiv5_swdp_scan(struct target_controller *tc, uint32_t targetid)
{
	target_list_free(&(tc->target_list));
	ADIv5_DP_t idp = {
		.dp_low_write = tc->swdptap_low_write,
		.error = firmware_swdp_error,
		.dp_read = firmware_swdp_read,
		.low_access = tc->swdptap_low_access,
		.abort = firmware_swdp_abort,
        .tc = tc
	};
	ADIv5_DP_t *initial_dp = &idp;
	if (swdptap_init(initial_dp))
		return -1;
    target_controller_t *initial_tc = initial_dp->tc;
	/* DORMANT-> SWD sequence*/
	initial_dp->seq_out(initial_tc, 0xFFFFFFFF, 32);
	initial_dp->seq_out(initial_tc, 0xFFFFFFFF, 32);
	/* 128 bit selection alert sequence for SW-DP-V2 */
	initial_dp->seq_out(initial_tc, 0x6209f392, 32);
	initial_dp->seq_out(initial_tc, 0x86852d95, 32);
	initial_dp->seq_out(initial_tc, 0xe3ddafe9, 32);
	initial_dp->seq_out(initial_tc, 0x19bc0ea2, 32);
	/* 4 cycle low,
	 * 0x1a Arm CoreSight SW-DP activation sequence
	 * 20 bits start of reset another reset sequence*/
	initial_dp->seq_out(initial_tc, 0x1a0, 12);
	uint32_t idcode = 0;
	volatile uint32_t target_id = 0;
	bool scan_multidrop = true;
	if (!targetid || !initial_dp->dp_low_write) {
		/* No targetID given on the command line or probe can not
		 * handle multi-drop. Try to read ID */
		dp_line_reset(initial_dp);
		volatile struct exception e;
		TRY_CATCH (e, EXCEPTION_ALL) {
			idcode = initial_dp->dp_read(initial_dp, ADIV5_DP_IDCODE);
		}
		if (e.type || initial_dp->fault)
        {
			scan_multidrop = false;
			DEBUG_WARN("Trying old JTAG to SWD sequence\n");
			initial_dp->seq_out(initial_tc, 0xFFFFFFFF, 32);
			initial_dp->seq_out(initial_tc, 0xFFFFFFFF, 32);
			initial_dp->seq_out(initial_tc, 0xE79E, 16); /* 0b0111100111100111 */
			dp_line_reset(initial_dp);
			initial_dp->fault = 0;
			volatile struct exception e2;
			TRY_CATCH (e2, EXCEPTION_ALL) {
				idcode = initial_dp->dp_read(initial_dp, ADIV5_DP_IDCODE);
			}
			if (e2.type || initial_dp->fault) {
				DEBUG_WARN("No usable DP found\n");
				return 0;
			}
		}
		if ((idcode & ADIV5_DP_VERSION_MASK) == ADIV5_DPv2) {
			scan_multidrop = true;
			/* Read TargetID. Can be done with device in WFI, sleep or reset!*/
			adiv5_dp_write(initial_dp, ADIV5_DP_SELECT, 2);
			target_id = adiv5_dp_read(initial_dp, ADIV5_DP_CTRLSTAT);
			adiv5_dp_write(initial_dp, ADIV5_DP_SELECT, 0);
			DEBUG_INFO("TARGETID %08" PRIx32 "\n", target_id);
			switch (target_id) {
			case 0x01002927: /* RP2040 */
				/* Release evt. handing RESCUE DP reset*/
				adiv5_dp_write(initial_dp, ADIV5_DP_CTRLSTAT, 0);
				break;
			}
			if (!initial_dp->dp_low_write) {
				DEBUG_WARN("CMSIS_DAP < V1.2 can not handle multi-drop!\n");
				/* E.g. CMSIS_DAP < V1.2 can not handle multi-drop!*/
				scan_multidrop = false;
			}
		} else {
			scan_multidrop = false;
		}
	} else {
		target_id = targetid;
	}
	volatile int nr_dps = (scan_multidrop) ? 16: 1;
	volatile uint32_t dp_targetid;
	for (volatile int i = 0; i < nr_dps; i++) {
		if (scan_multidrop) {
			dp_line_reset(initial_dp);
			dp_targetid = (i << 28) | (target_id & 0x0fffffff);
			initial_dp->dp_low_write(initial_dp, ADIV5_DP_TARGETSEL,
									dp_targetid);
			volatile struct exception e;
			TRY_CATCH (e, EXCEPTION_ALL) {
				idcode = initial_dp->dp_read(initial_dp, ADIV5_DP_IDCODE);
			}
			if (e.type || initial_dp->fault) {
				continue;
			}
		} else {
			dp_targetid = target_id;
		}
		ADIv5_DP_t *dp = MemManager_Alloc(sizeof(*dp));
		if (!dp) {			/* MemManager_Alloc failed: heap exhaustion */
			DEBUG_WARN("MemManager_Alloc: failed in %s\n", __func__);
			continue;
		}
		memcpy(dp, initial_dp, sizeof(ADIv5_DP_t));
		dp->idcode = idcode;
		dp->targetid = dp_targetid;
        dp->tc = tc;
		adiv5_dp_init(dp);
	}
	return tc->target_list ? 1 : 0;
}

uint32_t firmware_swdp_read(ADIv5_DP_t *dp, uint16_t addr)
{
	if (addr & ADIV5_APnDP) {
		adiv5_dp_low_access(dp, ADIV5_LOW_READ, addr, 0);
		return adiv5_dp_low_access(dp, ADIV5_LOW_READ,
		                           ADIV5_DP_RDBUFF, 0);
	} else {
		return adiv5_dp_low_access(dp, ADIV5_LOW_READ, addr, 0);
	}
}

 uint32_t firmware_swdp_error(ADIv5_DP_t *dp)
{
	if ((dp->fault && (dp->idcode & ADIV5_DP_VERSION_MASK) == ADIV5_DPv2) &&
		dp->dp_low_write) {
		/* On protocoll error target gets deselected.
		 * With DP Change, another target needs selection.
		 * => Reselect with right target! */
		dp_line_reset(dp);
		dp->dp_low_write(dp, ADIV5_DP_TARGETSEL, dp->targetid);
		dp->dp_read(dp, ADIV5_DP_IDCODE);
		/* Exception here is unexpected, so do not catch */
	}
	uint32_t err, clr = 0;
	err = adiv5_dp_read(dp, ADIV5_DP_CTRLSTAT) &
		(ADIV5_DP_CTRLSTAT_STICKYORUN | ADIV5_DP_CTRLSTAT_STICKYCMP |
		ADIV5_DP_CTRLSTAT_STICKYERR | ADIV5_DP_CTRLSTAT_WDATAERR);

	if(err & ADIV5_DP_CTRLSTAT_STICKYORUN)
		clr |= ADIV5_DP_ABORT_ORUNERRCLR;
	if(err & ADIV5_DP_CTRLSTAT_STICKYCMP)
		clr |= ADIV5_DP_ABORT_STKCMPCLR;
	if(err & ADIV5_DP_CTRLSTAT_STICKYERR)
		clr |= ADIV5_DP_ABORT_STKERRCLR;
	if(err & ADIV5_DP_CTRLSTAT_WDATAERR)
		clr |= ADIV5_DP_ABORT_WDERRCLR;

	adiv5_dp_write(dp, ADIV5_DP_ABORT, clr);
	dp->fault = 0;

	return err;
}

uint32_t firmware_swdp_low_access(ADIv5_DP_t *dp, uint8_t RnW,
				      uint16_t addr, uint32_t value) {
/*#if (USE_PIO != 0)
        uint32_t prog_offset = 0;
        pio_sm_config prog_config;

        uint32_t request = make_packet_request(RnW, addr) |  (1 << 9);//Set 9th bit to 1 to set final pindir to output
        uint32_t response = 0;
        uint32_t ack;
        platform_timeout timeout;

        pio_clear_instruction_memory(pio0);
        if (RnW)
        {
            prog_offset = pio_add_program(pio0, &swd_dp_low_access_read_program);
            prog_config = swd_dp_low_access_read_program_get_default_config(prog_offset);
        }
        else
        {
            prog_offset = pio_add_program(pio0, &swd_dp_low_access_write_program);
            prog_config = swd_dp_low_access_write_program_get_default_config(prog_offset);
        }

        pio_gpio_init(pio0, TARGET1_TMS_DIR_PIN);
        pio_gpio_init(pio0, TARGET1_TCK);
        pio_gpio_init(pio0, TARGET1_TMS);

        sm_config_set_out_pins(&prog_config, TARGET1_TMS, 1);
        sm_config_set_set_pins(&prog_config, TARGET1_TMS_DIR_PIN, 1);
        sm_config_set_in_pins(&prog_config, TARGET1_TMS);
        sm_config_set_sideset_pins(&prog_config, TARGET1_TCK);

        sm_config_set_out_shift(&prog_config, true, false, 32);
        sm_config_set_in_shift(&prog_config, true, false, 32);
        sm_config_set_clkdiv(&prog_config, dp->tc->target_interface_param.clkdiv);

        const uint32_t swd_all_pins = (1 << TARGET1_TMS) | (1 << TARGET1_TMS_DIR_PIN) | (1 << TARGET1_TCK);
        pio_sm_set_pins_with_mask(pio0, 0, (1 << TARGET1_TMS_DIR_PIN), swd_all_pins);
        pio_sm_set_pindirs_with_mask(pio0, 0, swd_all_pins, swd_all_pins);

        pio_sm_init(pio0, 0, prog_offset, &prog_config);
        pio_sm_set_enabled(pio0, 0, true);

        if ((addr & ADIV5_APnDP) && dp->fault) return 0;

        platform_timeout_set(&timeout, 20);
        do {
            pio_sm_put_blocking(pio0, 0, 0x20000000);
            pio_sm_put_blocking(pio0, 0, request);
            if (!RnW)
            {
                pio_sm_put_blocking(pio0, 0, value);
                int parity = __builtin_popcount(value);
                pio_sm_put_blocking(pio0, 0, parity & 1);
            }
            ack = (pio_sm_get_blocking(pio0, 0) >> 29);

            if (ack == SWDP_ACK_FAULT) {
                pio_sm_set_enabled(pio0, 0, false);
                dp->fault = 1;
                return 0;
            }
        } while (ack == SWDP_ACK_WAIT && !platform_timeout_is_expired(&timeout));

        if (ack == SWDP_ACK_WAIT) {
            pio_sm_set_enabled(pio0, 0, false);
            dp->abort(dp, ADIV5_DP_ABORT_DAPABORT);
            dp->fault = 1;
            return 0;
        }

        if (ack != SWDP_ACK_OK)
            raise_exception(EXCEPTION_ERROR, "SWDP invalid ACK");

        if (RnW) {
            response = pio_sm_get_blocking(pio0, 0);
            int parity = __builtin_popcount(response);
            parity += (pio_sm_get_blocking(pio0, 0) ? 1 : 0);
            //uint32_t parity_test = pio_sm_get_blocking(pio0, 0);
            //parity += parity_test;
            pio_sm_set_enabled(pio0, 0, false);
            if (parity & 1) {
                dp->fault = 1;
                raise_exception(EXCEPTION_ERROR, "SWDP Parity error");
            }
        }
        else
        {
            pio_sm_set_enabled(pio0, 0, false);
        }

        return response;
#else*/
        uint32_t request = make_packet_request(RnW, addr);
        uint32_t response = 0;
        uint32_t ack;
        platform_timeout timeout;

        if ((addr & ADIV5_APnDP) && dp->fault) return 0;

        platform_timeout_set(&timeout, 20);
        do {
            dp->seq_out(dp->tc, request, 8);
            ack = dp->seq_in(dp->tc, 3)  >> 29;
            if (ack == SWDP_ACK_FAULT) {
                dp->fault = 1;
                return 0;
            }
        } while (ack == SWDP_ACK_WAIT && !platform_timeout_is_expired(&timeout));

        if (ack == SWDP_ACK_WAIT) {
            dp->abort(dp, ADIV5_DP_ABORT_DAPABORT);
            dp->fault = 1;
            return 0;
        }

        if (ack == SWDP_ACK_FAULT) {
            dp->fault = 1;
            return 0;
        }

        if (ack != SWDP_ACK_OK)
            raise_exception(EXCEPTION_ERROR, "SWDP invalid ACK");

        if (RnW) {
            if (dp->seq_in_parity(dp->tc, &response, 32)) {
                dp->fault = 1;
                raise_exception(EXCEPTION_ERROR, "SWDP Parity error");
            }
        } else {
            dp->seq_out_parity(dp->tc, value, 32);
            dp->seq_out(dp->tc, 0, 8);
        }
        return response;
//#endif
}

void firmware_swdp_abort(ADIv5_DP_t *dp, uint32_t abort)
{
	adiv5_dp_write(dp, ADIV5_DP_ABORT, abort);
}