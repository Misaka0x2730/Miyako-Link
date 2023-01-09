#include "general.h"
#include "wizchip_conf.h"
#include "w5500_common.h"

uint8_t w5500_get_global_sockets_irq_status(void)
{
    return getSIR();
}

uint8_t w5500_get_global_sockets_irq_mask(void)
{
    return getSIMR();
}

void w5500_set_global_sockets_irq_mask(uint8_t irq_mask)
{
    setSIMR(irq_mask);
}

uint8_t w5500_read_byte_register(uint32_t addr)
{
    uint8_t reg = 0;

    WIZCHIP_READ_BUF(addr, &reg, sizeof(reg));

    return reg;
}

uint16_t w5500_read_two_byte_register(uint32_t addr)
{
    uint8_t reg[sizeof(uint16_t)] = { 0 };

    WIZCHIP_READ_BUF(addr, reg, sizeof(reg));

    return ((uint16_t)reg[0] << 8) + reg[1];
}

void w5500_write_byte_register(uint32_t addr, uint8_t data)
{
    WIZCHIP_WRITE_BUF(addr, &data, sizeof(data));
}

void w5500_write_two_byte_register(uint32_t addr, uint16_t data)
{
    uint8_t reg[sizeof(uint16_t)] = { 0 };

    reg[0] = (data >> 8) & 0xFF;
    reg[1] = data & 0xFF;

    WIZCHIP_WRITE_BUF(addr, reg, sizeof(reg));
}