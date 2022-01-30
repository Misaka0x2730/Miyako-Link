#ifndef __PLATFORM_TARGET_INTERFACE_NON_ISO_H
#define __PLATFORM_TARGET_INTERFACE_NON_ISO_H

#include "adiv5.h"
#include "target.h"

const char *platform_target_interface_non_iso_target_voltage(void);
void platform_target_interface_non_iso_srst_set_val(bool assert);
bool platform_target_interface_non_iso_srst_get_val(void);
void platform_target_interface_non_iso_set_power(bool power);
bool platform_target_interface_non_iso_get_power(void);
void platform_target_interface_non_iso_set_idle_state(bool state);
void platform_target_interface_non_iso_set_error_state(bool state);

void platform_target_interface_non_iso_seq_out(struct target_controller *tc, uint32_t MS, int ticks);
void platform_target_interface_non_iso_seq_out_parity(struct target_controller *tc, uint32_t MS, int ticks);
uint32_t platform_target_interface_non_iso_seq_in(struct target_controller *tc, int ticks);
bool platform_target_interface_non_iso_seq_in_parity(struct target_controller *tc, uint32_t *ret, int ticks);

bool platform_target_interface_non_iso_swd_low_write(ADIv5_DP_t *dp, uint16_t addr, const uint32_t data);
uint32_t platform_target_interface_non_iso_swd_low_access(ADIv5_DP_t *dp, uint8_t RnW,
                                                          uint16_t addr, uint32_t value);

uint8_t platform_target_interface_non_iso_jtag_next(struct target_controller *tc, uint8_t dTMS, uint8_t dTDI);
void platform_target_interface_non_iso_jtag_tms_seq(struct target_controller *tc, uint32_t MS, int ticks);
void platform_target_interface_non_iso_jtag_tdi_tdo_seq(struct target_controller *tc, uint8_t *DO, const uint8_t final_tms, const uint8_t *DI, int ticks);
void platform_target_interface_non_iso_jtag_tdi_seq(struct target_controller *tc, const uint8_t final_tms, const uint8_t *DI, int ticks);

#endif // __PLATFORM_TARGET_INTERFACE_NON_ISO_H
