#include "general.h"
#include "exception.h"

#include "platform.h"
#include "device_config.h"

#include "hardware/pio.h"
#include "target_interface_non_iso_swd_low_access_read.pio.h"
#include "target_interface_non_iso_swd_low_access_write.pio.h"
#include "target_interface_non_iso_swd_low_seq_in.pio.h"
#include "target_interface_non_iso_swd_low_seq_out.pio.h"
#include "target_interface_non_iso_swd_low_write.pio.h"

#include "system_pins.h"
#include "platform_target_interface_non_iso.h"

static void platorm_pio_non_iso_prog_config(pio_sm_config *prog_config, float clkdiv);
static void platorm_pio_non_iso_pins_config(PIO pio, uint32_t sm);

static void platform_pio_non_iso_seq_out(struct target_controller *tc, uint32_t MS, int ticks, bool isParity, uint32_t parity);
static void platform_pio_non_iso_seq_in(struct target_controller *tc, int ticks, uint32_t *pData, uint32_t *pParity);

const char *platform_target_interface_non_iso_target_voltage(void)
{
    return "UNKNOWN";
}

void platform_target_interface_non_iso_srst_set_val(bool assert)
{
    system_pin_set_value(PIN_TARGET_1_RST, assert);
    platform_delay(1);
}

bool platform_target_interface_non_iso_srst_get_val(void)
{
	return system_pin_get(PIN_TARGET_1_RST);
}

void platform_target_interface_non_iso_set_power(bool power)
{
    system_pin_set_value(PIN_TARGET_1_PWR_EN, !power);
}

bool platform_target_interface_non_iso_get_power(void)
{
    return (system_pin_get(PIN_TARGET_1_PWR_EN) == false);
}

void platform_target_interface_non_iso_set_idle_state(bool state)
{
    system_pin_set_value(PIN_TARGET_1_LED_ACT, state);
}

void platform_target_interface_non_iso_set_error_state(bool state)
{
    system_pin_set_value(PIN_TARGET_1_LED_ERR, state);
}

static void platorm_pio_non_iso_prog_config(pio_sm_config *prog_config, float clkdiv)
{
    sm_config_set_in_pins(prog_config, PIN_TARGET1_TMS);
    sm_config_set_out_pins(prog_config, PIN_TARGET1_TMS, 1);
    sm_config_set_set_pins(prog_config, PIN_TARGET1_TMS_DIR_PIN, 1);
    sm_config_set_sideset_pins(prog_config, PIN_TARGET1_TCK);

    sm_config_set_out_shift(prog_config, true, false, 32);
    sm_config_set_in_shift(prog_config, true, true, 32);
    sm_config_set_clkdiv(prog_config, clkdiv);
}

static void platorm_pio_non_iso_pins_config(PIO pio, uint32_t sm)
{
    const uint32_t swd_all_pins = (1 << PIN_TARGET1_TMS) | (1 << PIN_TARGET1_TMS_DIR_PIN) | (1 << PIN_TARGET1_TCK);
    pio_sm_set_pins_with_mask(pio, sm, 0, swd_all_pins);
    pio_sm_set_pindirs_with_mask(pio, sm, (1 << PIN_TARGET1_TMS_DIR_PIN) | (1 << PIN_TARGET1_TCK), swd_all_pins);

    pio_gpio_init(pio, PIN_TARGET1_TMS_DIR_PIN);
    pio_gpio_init(pio, PIN_TARGET1_TCK);
    pio_gpio_init(pio, PIN_TARGET1_TMS);
}

static void platform_pio_non_iso_seq_out(struct target_controller *tc, uint32_t MS, int ticks, bool isParity, uint32_t parity)
{
    if (ticks == 0)
    {
        return;
    }

    target_interface_tms_dir_t olddir = tc->target_interface_param.tms_direction;

    pio_sm_set_enabled(pio0, 0, false);

    if (tc->target_interface_param.current_program != PIO_PROGRAM_SWD_DP_LOW_SEQ_OUT) {
        tc->target_interface_param.current_program = PIO_PROGRAM_SWD_DP_LOW_SEQ_OUT;
        pio_clear_instruction_memory(pio0);
        int prog_offset = pio_add_program(pio0, &target_interface_non_iso_swd_low_seq_out_program);
        pio_sm_config prog_config = target_interface_non_iso_swd_low_seq_out_program_get_default_config(prog_offset);

        platorm_pio_non_iso_prog_config(&prog_config, tc->target_interface_param.clkdiv);
        tc->target_interface_param.update_clkdiv = false;

        if (tc->target_interface_param.tms_direction == TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED)
        {
            platorm_pio_non_iso_pins_config(pio0, 0);
        }

        pio_sm_init(pio0, 0, prog_offset, &prog_config);
    }

    if (tc->target_interface_param.update_clkdiv) {
        pio_sm_set_clkdiv(pio0, 0, tc->target_interface_param.clkdiv);
        pio_sm_clkdiv_restart(pio0, 0);

        tc->target_interface_param.update_clkdiv = false;
    }

    tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_OUT;

    pio_sm_clear_fifos(pio0, 0);
    pio_sm_put_blocking(pio0, 0, (olddir != TARGET_INTERFACE_TMS_DIR_OUT) ? 1 : 0);
    pio_sm_put_blocking(pio0, 0, ticks > 32 ? 32 - 1 : ticks - 1);
    pio_sm_put_blocking(pio0, 0, MS);
    if (isParity) {
        parity = (parity << 1) | (1 << 0);
    }
    else {
        parity = 0;
    }
    pio_sm_put_blocking(pio0, 0, parity);

    pio_sm_set_enabled(pio0, 0, true);
    pio_sm_get_blocking(pio0, 0);
    pio_sm_set_enabled(pio0, 0, false);
}

static void platform_pio_non_iso_seq_in(struct target_controller *tc, int ticks, uint32_t *pData, uint32_t *pParity)
{
    if ((ticks == 0) || (pData == NULL))
    {
        return;
    }

    target_interface_tms_dir_t olddir = tc->target_interface_param.tms_direction;

    pio_sm_set_enabled(pio0, 0, false);
    if (tc->target_interface_param.current_program != PIO_PROGRAM_SWD_DP_LOW_SEQ_IN) {
        tc->target_interface_param.current_program = PIO_PROGRAM_SWD_DP_LOW_SEQ_IN;
        pio_clear_instruction_memory(pio0);
        int prog_offset = pio_add_program(pio0, &target_interface_non_iso_swd_low_seq_in_program);
        pio_sm_config prog_config = target_interface_non_iso_swd_low_seq_in_program_get_default_config(prog_offset);

        platorm_pio_non_iso_prog_config(&prog_config, tc->target_interface_param.clkdiv);
        tc->target_interface_param.update_clkdiv = false;

        if (tc->target_interface_param.tms_direction == TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED)
        {
            platorm_pio_non_iso_pins_config(pio0, 0);
        }

        pio_sm_init(pio0, 0, prog_offset, &prog_config);
    }

    if (tc->target_interface_param.update_clkdiv) {
        pio_sm_set_clkdiv(pio0, 0, tc->target_interface_param.clkdiv);
        pio_sm_clkdiv_restart(pio0, 0);

        tc->target_interface_param.update_clkdiv = false;
    }

    tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_IN;

    pio_sm_clear_fifos(pio0, 0);
    pio_sm_put_blocking(pio0, 0, ((olddir != TARGET_INTERFACE_TMS_DIR_IN) && (olddir != TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED)) ? 1 : 0);
    pio_sm_put_blocking(pio0, 0, ticks > 32 ? 32 - 1 : ticks - 1);
    pio_sm_put_blocking(pio0, 0, (pParity != NULL) ? 0x03 : 0x00);

    pio_sm_set_enabled(pio0, 0, true);
    *pData = pio_sm_get_blocking(pio0, 0);
    if (pParity != NULL) {
        *pParity = pio_sm_get_blocking(pio0, 0);
    }
    pio_sm_get_blocking(pio0, 0);
    if (pParity != NULL) {
        tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_OUT;
    }
    pio_sm_set_enabled(pio0, 0, false);
}

void platform_target_interface_non_iso_seq_out(struct target_controller *tc, uint32_t MS, int ticks)
{
    platform_pio_non_iso_seq_out(tc, MS, ticks, false, 0);
}

void platform_target_interface_non_iso_seq_out_parity(struct target_controller *tc, uint32_t MS, int ticks)
{
    int parity = __builtin_popcount(MS);
    platform_pio_non_iso_seq_out(tc, MS, ticks, true, parity & 1);
}

uint32_t platform_target_interface_non_iso_seq_in(struct target_controller *tc, int ticks)
{
    uint32_t ret = 0;
    platform_pio_non_iso_seq_in(tc, ticks, &ret, NULL);

    return ret;
}

bool platform_target_interface_non_iso_seq_in_parity(struct target_controller *tc, uint32_t *ret, int ticks)
{
    uint32_t parity = 0;
    platform_pio_non_iso_seq_in(tc, ticks, ret, &parity);

    parity = (parity) ? (__builtin_popcount(*ret) + 1) : __builtin_popcount(*ret);

    return (parity & 1) ? true : false;
}

bool platform_target_interface_non_iso_swd_low_write(ADIv5_DP_t *dp, uint16_t addr, const uint32_t data)
{
    unsigned int request = make_packet_request(ADIV5_LOW_WRITE, addr & 0xf);
    request |= (1 << 9);
    uint32_t res = 0;
    uint32_t ack;
    platform_timeout timeout;
    struct target_controller *tc = dp->tc;

    target_interface_tms_dir_t olddir = tc->target_interface_param.tms_direction;

    pio_sm_set_enabled(pio0, 0, false);
    if (tc->target_interface_param.current_program != PIO_PROGRAM_SWD_DP_LOW_WRITE) {
        tc->target_interface_param.current_program = PIO_PROGRAM_SWD_DP_LOW_WRITE;

        pio_clear_instruction_memory(pio0);
        int prog_offset = pio_add_program(pio0, &target_interface_non_iso_swd_low_write_program);
        pio_sm_config prog_config = target_interface_non_iso_swd_low_write_program_get_default_config(prog_offset);

        platorm_pio_non_iso_prog_config(&prog_config, tc->target_interface_param.clkdiv);
        tc->target_interface_param.update_clkdiv = false;

        if (tc->target_interface_param.tms_direction == TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED) {
            platorm_pio_non_iso_pins_config(pio0, 0);
        }

        pio_sm_init(pio0, 0, prog_offset, &prog_config);
    }

    if (tc->target_interface_param.update_clkdiv) {
        pio_sm_set_clkdiv(pio0, 0, tc->target_interface_param.clkdiv);
        pio_sm_clkdiv_restart(pio0, 0);

        tc->target_interface_param.update_clkdiv = false;
    }

    tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_OUT;

    const int parity = __builtin_popcount(data);

    pio_sm_clear_fifos(pio0, 0);
    pio_sm_put_blocking(pio0, 0, (olddir != TARGET_INTERFACE_TMS_DIR_OUT) ? 1 : 0);
    pio_sm_put_blocking(pio0, 0, request);
    pio_sm_put_blocking(pio0, 0, data);
    pio_sm_put_blocking(pio0, 0, parity & 1);

    pio_sm_set_enabled(pio0, 0, true);
    res = (pio_sm_get_blocking(pio0, 0) >> 29);
    pio_sm_set_enabled(pio0, 0, false);

    return (res != SWDP_ACK_OK);
}

uint32_t platform_target_interface_non_iso_swd_low_access(ADIv5_DP_t *dp, uint8_t RnW,
                                                          uint16_t addr, uint32_t value)
{
    uint32_t prog_offset = 0;
    pio_sm_config prog_config;

    uint32_t request = make_packet_request(RnW, addr) |  (1 << 9);//Set 9th bit to 1 to set final pindir to output
    uint32_t response = 0;
    uint32_t ack;
    platform_timeout timeout;
    struct target_controller *tc = dp->tc;

    target_interface_tms_dir_t olddir = tc->target_interface_param.tms_direction;

    if ((addr & ADIV5_APnDP) && dp->fault) return 0;

    pio_sm_set_enabled(pio0, 0, false);

    const target_interface_pio_program_t target_program = (RnW) ? PIO_PROGRAM_SWD_DP_LOW_ACCESS_READ : PIO_PROGRAM_SWD_DP_LOW_ACCESS_WRITE;
    if (tc->target_interface_param.current_program != target_program)
    {
        pio_clear_instruction_memory(pio0);
        if (RnW) {
            tc->target_interface_param.current_program = PIO_PROGRAM_SWD_DP_LOW_ACCESS_READ;
            prog_offset = pio_add_program(pio0, &target_interface_non_iso_swd_low_access_read_program);
            prog_config = target_interface_non_iso_swd_low_access_read_program_get_default_config(prog_offset);
        } else {
            tc->target_interface_param.current_program = PIO_PROGRAM_SWD_DP_LOW_ACCESS_WRITE;
            prog_offset = pio_add_program(pio0, &target_interface_non_iso_swd_low_access_write_program);
            prog_config = target_interface_non_iso_swd_low_access_write_program_get_default_config(prog_offset);
        }

        platorm_pio_non_iso_prog_config(&prog_config, tc->target_interface_param.clkdiv);
        tc->target_interface_param.update_clkdiv = false;

        if (tc->target_interface_param.tms_direction == TARGET_INTERFACE_TMS_DIR_NOT_INITIALIZED) {
            platorm_pio_non_iso_pins_config(pio0, 0);
        }

        pio_sm_init(pio0, 0, prog_offset, &prog_config);
    }

    if (tc->target_interface_param.update_clkdiv) {
        pio_sm_set_clkdiv(pio0, 0, tc->target_interface_param.clkdiv);
        pio_sm_clkdiv_restart(pio0, 0);

        tc->target_interface_param.update_clkdiv = false;
    }

    tc->target_interface_param.tms_direction = TARGET_INTERFACE_TMS_DIR_OUT;

    pio_sm_clear_fifos(pio0, 0);
    pio_sm_set_enabled(pio0, 0, true);

    if (olddir != TARGET_INTERFACE_TMS_DIR_OUT)
    {
        pio_sm_set_pins_with_mask(pio0, 0, (1 << PIN_TARGET1_TMS_DIR_PIN), (1 << PIN_TARGET1_TMS_DIR_PIN));
        pio_sm_set_pindirs_with_mask(pio0, 0, (1 << PIN_TARGET1_TMS), (1 << PIN_TARGET1_TMS));
    }
    platform_timeout_set(&timeout, 20);
    do {
        pio_sm_clear_fifos(pio0, 0);
        //pio_sm_put_blocking(pio0, 0, (olddir != TARGET_INTERFACE_TMS_DIR_OUT) ? 1 : 0);
        pio_sm_put_blocking(pio0, 0, 0x20000000);
        pio_sm_put_blocking(pio0, 0, request);

        if (!RnW)
        {
            int parity = __builtin_popcount(value);
            pio_sm_put_blocking(pio0, 0, value);
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

    if (ack != SWDP_ACK_OK) {
        raise_exception(EXCEPTION_ERROR, "SWDP invalid ACK");
    }

    if (RnW) {
        response = pio_sm_get_blocking(pio0, 0);
        int parity = __builtin_popcount(response);
        parity += (pio_sm_get_blocking(pio0, 0) ? 1 : 0);
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
}