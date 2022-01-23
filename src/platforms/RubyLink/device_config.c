#include "eeprom.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "system_func.h"
#include "device_config.h"
#include "platform.h"

device_conf_t device_configuration;
static SemaphoreHandle_t device_config_mutex;

static const uint8_t default_ip[4] = {192, 168, 0, 2};
static const uint8_t default_nm[4] = {255, 255, 255, 0};
static const uint8_t default_gw[4] = {192, 168, 0, 1};

static void device_config_lock(void);
static void device_config_unlock(void);

static void device_config_lock(void)
{
    xSemaphoreTakeRecursive(device_config_mutex, SYSTEM_WAIT_FOREVER);
}

static void device_config_unlock(void)
{
    xSemaphoreGiveRecursive(device_config_mutex);
}

static uint16_t crc16_function(const uint8_t *buf, uint32_t len)
{
    static const uint16_t crc_table[256] = {
            0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
            0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
            0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
            0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
            0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
            0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
            0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
            0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
            0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
            0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
            0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
            0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
            0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
            0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
            0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
            0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
            0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
            0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
            0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
            0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
            0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
            0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
            0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
            0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
            0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
            0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
            0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
            0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
            0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
            0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
            0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
            0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

    uint8_t xor = 0;
    uint16_t crc = 0xFFFF;

    while(len--)
    {
        xor = (*buf++) ^ crc;
        crc >>= 8;
        crc ^= crc_table[xor];
    }

    return crc;
}

static uint16_t device_config_calculate_crc(void)
{
    device_config_lock();
    const uint16_t crc =  crc16_function((uint8_t*)&(device_configuration.dhcp_enabled), sizeof(device_configuration) - sizeof(device_configuration.mac) - sizeof(device_configuration.crc));
    device_config_unlock();

    return crc;
}

void device_config_init(void)
{
    eeprom_init();

    device_config_mutex = xSemaphoreCreateRecursiveMutex();

    device_config_lock();

    //device_config_reset_to_default();
    //device_config_write_all_settings();
    if (device_config_read_all_settings())
    {
        //TODO: assert
    }

    uint16_t crc = 0;
    if (device_config_get_value(DEVICE_CONF_CRC, (uint8_t*)&crc, sizeof(crc)))
    {
        //TODO: assert
    }

    if (crc != device_config_calculate_crc())
    {
        device_config_reset_to_default();
        device_config_write_all_settings();
    }

    device_config_unlock();
}

void device_config_reset_to_default(void)
{
    device_config_lock();

    device_configuration.dhcp_enabled = 0;
    memcpy(device_configuration.ip, default_ip, sizeof(device_configuration.ip));
    memcpy(device_configuration.nm, default_nm, sizeof(device_configuration.nm));
    memcpy(device_configuration.gw, default_gw, sizeof(device_configuration.gw));
    memset(device_configuration.leds_enabled, 0xFF, sizeof(device_configuration.leds_enabled)); //All LEDs are enabled
    memset(device_configuration.interface_config, 0, sizeof(device_configuration.interface_config));

    device_configuration.target_interface_conf[0].frequency = TARGET_INTERFACE_1_DEFAULT_FREQUENCY;
    device_configuration.target_interface_conf[1].frequency = TARGET_INTERFACE_2_DEFAULT_FREQUENCY;
    // MAC Address should be read from EEPROM

    device_config_unlock();
}

int32_t device_config_read_all_settings(void)
{
    uint8_t reg = EEPROM_ADDR_START;
    uint16_t size = sizeof(device_configuration) - sizeof(device_configuration.mac); //MAC is stored at the end of EEPROM
    int32_t read_result = 0;

    device_config_lock();

    // Read configuration
    if (eeprom_read(reg, (uint8_t*)&device_configuration, size) < 0)
    {
        device_config_unlock();
        return -1;
    }

    // Read MAC address
    reg = EEPROM_ADDR_MAC;
    size = sizeof(device_configuration.mac);
    if (eeprom_read(reg, device_configuration.mac, size) < 0)
    {
        device_config_unlock();
        return -1;
    }

    device_config_unlock();

    return 0;
}

int32_t device_config_write_all_settings(void)
{
    device_config_lock();
    device_configuration.crc = device_config_calculate_crc();
    int32_t write_result = eeprom_write(EEPROM_ADDR_START, (uint8_t*)&device_configuration, sizeof(device_configuration) - sizeof(device_configuration.mac));
    device_config_unlock();

    if (write_result < 0)
    {
        //TODO: I2C EEPROM was lost, assert
    }

    return write_result;
}

static uint32_t device_config_get_setting_pointer(const device_conf_id_t id, uint8_t **pData)
{
    if (pData == NULL)
    {
        return 0;
    }

    uint32_t size = 0;
    switch (id)
    {
        case DEVICE_CONF_CRC:
            *pData = (uint8_t*)(&(device_configuration.crc));
            size = sizeof(device_configuration.crc);
            break;
        case DEVICE_CONF_DHCP_ENABLED:
            *pData = (uint8_t*)(&(device_configuration.dhcp_enabled));
            size = sizeof(device_configuration.dhcp_enabled);
            break;
        case DEVICE_CONF_IP:
            *pData = (uint8_t*)(&(device_configuration.ip));
            size = sizeof(device_configuration.ip);
            break;
        case DEVICE_CONF_NETMASK:
            *pData = (uint8_t*)(&(device_configuration.nm));
            size = sizeof(device_configuration.nm);
            break;
        case DEVICE_CONF_GATEWAY:
            *pData = (uint8_t*)(&(device_configuration.gw));
            size = sizeof(device_configuration.gw);
            break;
        case DEVICE_CONF_MAC:
            *pData = (uint8_t*)(&(device_configuration.mac));
            size = sizeof(device_configuration.mac);
            break;
        case DEVICE_CONF_LEDS_ENABLED:
            *pData = (uint8_t*)(&(device_configuration.leds_enabled));
            size = sizeof(device_configuration.leds_enabled);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_1:
            *pData = (uint8_t*)(&(device_configuration.interface_config[0]));
            size = sizeof(device_configuration.interface_config[0]);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_1_TYPE:
            *pData = (uint8_t*)(&(device_configuration.interface_config[0].interface_type));
            size = sizeof(device_configuration.interface_config[0].interface_type);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_1_PORT:
            *pData = (uint8_t*)(&(device_configuration.interface_config[0].port));
            size = sizeof(device_configuration.interface_config[0].port);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_2:
            *pData = (uint8_t*)(&(device_configuration.interface_config[1]));
            size = sizeof(device_configuration.interface_config[1]);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_2_TYPE:
            *pData = (uint8_t*)(&(device_configuration.interface_config[1].interface_type));
            size = sizeof(device_configuration.interface_config[1].interface_type);
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_2_PORT:
            *pData = (uint8_t*)(&(device_configuration.interface_config[1].port));
            size = sizeof(device_configuration.interface_config[1].port);
            break;
        case DEVICE_CONF_TARGET_INTERFACE_CONFIG_1:
            *pData = (uint8_t*)(&(device_configuration.target_interface_conf[0]));
            size = sizeof(device_configuration.target_interface_conf[0]);
            break;
        case DEVICE_CONF_TARGET_INTERFACE_CONFIG_1_FREQ:
            *pData = (uint8_t*)(&(device_configuration.target_interface_conf[0]).frequency);
            size = sizeof(device_configuration.target_interface_conf[0].frequency);
            break;
        case DEVICE_CONF_TARGET_INTERFACE_CONFIG_2:
            *pData = (uint8_t*)(&(device_configuration.target_interface_conf[1]));
            size = sizeof(device_configuration.target_interface_conf[1]);
            break;
        case DEVICE_CONF_TARGET_INTERFACE_CONFIG_2_FREQ:
            *pData = (uint8_t*)(&(device_configuration.target_interface_conf[1]).frequency);
            size = sizeof(device_configuration.target_interface_conf[1].frequency);
            break;
        default:
            *pData = NULL;
            break;
    }

    return size;
}

int32_t device_config_get_value(const device_conf_id_t id, uint8_t *pValue, const size_t size)
{
    int32_t retVal = 0;
    uint8_t *pData = NULL;

    if ((size == 0) || (id > DEVICE_CONF_MAX_ID))
    {
        return -1;
    }

    device_config_lock();

    const uint32_t setting_size = device_config_get_setting_pointer(id, &pData);
    if (setting_size == size)
    {
        memcpy(pValue, pData, size);
        retVal = (int32_t)size;
    }
    else
    {
        retVal = -1;
    }

    device_config_unlock();

    return retVal;
}

int32_t device_config_set_value(const device_conf_id_t id, uint8_t *pValue, const size_t size)
{
    int32_t retVal = 0;
    uint8_t *pData = NULL;
    if ((size == 0) || (id > DEVICE_CONF_MAX_ID))
    {
        return -1;
    }

    device_config_lock();
    const uint32_t setting_size = device_config_get_setting_pointer(id, &pData);

    if (setting_size == size)
    {
        if (memcmp(pData, pValue, size) != 0)
        {
            memcpy(pData, pValue, size);
            retVal = size;
        }
        else
        {
            retVal = 0;
        }
    }
    else
    {
        retVal = -1;
    }

    if (retVal > 0)
    {
        device_config_write_all_settings();
    }
    else if (retVal == 0)
    {
        retVal = (int32_t)size;
    }
    device_config_unlock();

    return retVal;
}