#ifndef _W5500_COMMON_H
#define _W5500_COMMON_H

#include "wizchip_conf.h"

#define W5500_SOCK_AMOUNT           (_WIZCHIP_SOCK_NUM_)
#define W5500_SOCK_UNUSED           (UINT8_MAX)
#define W5500_SOCK_USED             (W5500_SOCK_UNUSED - 1)

#define W5500_SOCK_RX_BUF_SIZE      (4096)
#define W5500_TX_MAX_ATTEMPT_COUNT  (5)

uint8_t w5500_get_global_sockets_irq_status(void);
uint8_t w5500_get_global_sockets_irq_mask(void);
void w5500_set_global_sockets_irq_mask(uint8_t irq_mask);

uint8_t w5500_read_byte_register(uint32_t addr);
uint16_t w5500_read_two_byte_register(uint32_t addr);
void w5500_write_byte_register(uint32_t addr, uint8_t data);
void w5500_write_two_byte_register(uint32_t addr, uint16_t data);

#endif //_W5500_COMMON_H
