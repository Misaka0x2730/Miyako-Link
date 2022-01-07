#include "eeprom.h"

void eeprom_init(void)
{
    i2c_init(EEPROM_I2C, EEPROM_I2C_BAUD);
    gpio_set_function(EEPROM_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(EEPROM_PIN_SCL, GPIO_FUNC_I2C);
}

int32_t eeprom_read(const uint8_t address, uint8_t *pData, const size_t length)
{
    if (i2c_write_blocking(EEPROM_I2C, EEPROM_ADDR, &address, sizeof(address), true) < 0)
    {
        return -1;
    }

    int read_len = i2c_read_blocking(EEPROM_I2C,  EEPROM_ADDR, pData, length, false);

    if (read_len < 0)
    {
        return -1;
    }

    return read_len;
}

int32_t eeprom_write(const uint8_t address, uint8_t *pData, const size_t length)
{
    uint8_t data[EEPROM_MAX_WRITE_SIZE + 1] = { 0 }; // Only 8 bytes can be written in one transfer
    uint8_t attempt_count = 0;

    if ((length > EEPROM_LAST_ADDR) || (((uint16_t)address + length) > EEPROM_LAST_ADDR))
    {
        return -1;
    }

    for (size_t i = 0; i < length; i += EEPROM_MAX_WRITE_SIZE)
    {
        const uint16_t write_size = (length - i) > EEPROM_MAX_WRITE_SIZE ? EEPROM_MAX_WRITE_SIZE + 1 : (length - i) + 1; //Address byte is also counted
        data[0] = address + i; // Set address
        memcpy(&(data[1]), pData + i, write_size);
        attempt_count = 0;
        while ((i2c_write_blocking(EEPROM_I2C, EEPROM_ADDR, data, write_size, false) < 0) && (attempt_count < EEPROM_WR_ATTEMPTS))
        {
            busy_wait_ms(EEPROM_WR_DELAY);
            attempt_count++;
        }

        if (attempt_count >= EEPROM_WR_ATTEMPTS)
        {
            return -1;
        }
    }

    return length;
}