#ifndef __JTAG_DEV_H
#define __JTAG_DEV_H

#define JTAG_MAX_DEVS	32
#define JTAG_MAX_IR_LEN	16

typedef struct jtag_dev_s {
    union {
        uint8_t jd_dev;
        uint8_t dr_prescan;
    };
    uint8_t dr_postscan;

    uint8_t ir_len;
    uint8_t ir_prescan;
    uint8_t ir_postscan;
    uint32_t jd_idcode;
    const char *jd_descr;
    uint32_t current_ir;

    struct jtag_proc_s *jtag_proc;
} jtag_dev_t;

#endif // __JTAG_DEV_H