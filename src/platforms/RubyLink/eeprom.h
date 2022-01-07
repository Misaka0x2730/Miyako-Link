#ifndef __EEPROM_H
#define __EEPROM_H

#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#define EEPROM_I2C              (i2c1)
#define EEPROM_I2C_BAUD         (20000)

#define EEPROM_PIN_SDA          (6)
#define EEPROM_PIN_SCL          (7)

#define EEPROM_ADDR             (0x50) //24AA02E48 address
#define EEPROM_ADDR_START       (0x00)
#define EEPROM_ADDR_MAC         (0xFA) //MAC address is stored in 0xFA - 0xFF
#define EEPROM_LAST_ADDR        (EEPROM_ADDR_MAC)

#define EEPROM_WR_DELAY         (5) // 5 ms before next writing attempt
#define EEPROM_WR_ATTEMPTS      (100) // 5*100 = 500 ms

#define EEPROM_MAX_WRITE_SIZE   (8)

void eeprom_init(void);
int32_t eeprom_read(const uint8_t address, uint8_t *pData, const size_t length);
int32_t eeprom_write(const uint8_t address, uint8_t *pData, const size_t length);

#endif //__EEPROM_H