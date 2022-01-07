#include "eeprom.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "system_func.h"
#include "device_config.h"

device_conf_t device_configuration;
static SemaphoreHandle_t device_config_mutex;

static const uint8_t default_ip[4] = {192, 168, 0, 2};
static const uint8_t default_nm[4] = {255, 255, 255, 0};
static const uint8_t default_gw[4] = {192, 168, 0, 1};

static void device_config_lock(void);
static void device_config_unlock(void);

static void device_config_lock(void)
{
    xSemaphoreTake(device_config_mutex, SYSTEM_WAIT_FOREVER);
}

static void device_config_unlock(void)
{
    xSemaphoreGive(device_config_mutex);
}

void device_config_init(void)
{
    eeprom_init();

    device_config_mutex = xSemaphoreCreateRecursiveMutex();

    device_config_lock();

    device_config_reset_to_default();
    device_config_write_all_settings();
    if (device_config_read_all_settings())
    {
        //TODO: assert
    }

    uint16_t settings_present = 0;
    if (device_config_get_value(DEVICE_CONF_SETTINGS_PRESENT, (uint8_t*)&settings_present, sizeof(settings_present)))
    {
        //TODO: assert
    }

    if (settings_present != SETTINGS_PRESENT)
    {
        device_config_reset_to_default();
        device_config_write_all_settings();
    }

    device_config_unlock();
}

void device_config_reset_to_default(void)
{
    device_config_lock();

    device_configuration.settings_present = SETTINGS_PRESENT;
    device_configuration.dhcp_enabled = 0;
    memcpy(device_configuration.ip, default_ip, sizeof(device_configuration.ip));
    memcpy(device_configuration.nm, default_nm, sizeof(device_configuration.nm));
    memcpy(device_configuration.gw, default_gw, sizeof(device_configuration.gw));
    memset(device_configuration.leds_enabled, 0xFF, sizeof(device_configuration.leds_enabled)); //All LEDs are enabled
    memset(device_configuration.interface_config, 0, sizeof(device_configuration.interface_config));
    // MAC Address should be read from EEPROM

    device_config_unlock();
}

int16_t device_config_read_all_settings(void)
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

int16_t device_config_write_all_settings(void)
{
    device_config_lock();
    int32_t write_result = eeprom_write(EEPROM_ADDR_START, (uint8_t*)&device_configuration, sizeof(device_configuration) - sizeof(device_configuration.mac));
    device_config_unlock();

    if (write_result < 0)
    {
        //TODO: I2C EEPROM was lost, assert
    }

    return write_result;
}

int16_t device_config_get_value(const device_conf_id_t id, uint8_t *pValue, const uint16_t size)
{
    int16_t retVal = 0;
    device_config_lock();
    switch (id)
    {
        case DEVICE_CONF_SETTINGS_PRESENT:
            if (size == sizeof(device_configuration.settings_present))
            {
                memcpy(pValue, &(device_configuration.settings_present), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_DHCP_ENABLED:
            if (size == sizeof(device_configuration.dhcp_enabled))
            {
                memcpy(pValue, &(device_configuration.dhcp_enabled), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_IP:
            if (size == sizeof(device_configuration.ip))
            {
                memcpy(pValue, &(device_configuration.ip), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_NETMASK:
            if (size == sizeof(device_configuration.nm))
            {
                memcpy(pValue, &(device_configuration.nm), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_GATEWAY:
            if (size == sizeof(device_configuration.gw))
            {
                memcpy(pValue, &(device_configuration.gw), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_MAC:
            if (size == sizeof(device_configuration.mac))
            {
                memcpy(pValue, &(device_configuration.mac), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_LEDS_ENABLED:
            if (size == sizeof(device_configuration.leds_enabled))
            {
                memcpy(pValue, &(device_configuration.leds_enabled), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_1:
            if (size == sizeof(device_configuration.interface_config[0]))
            {
                memcpy(pValue, &(device_configuration.interface_config[0]), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_2:
            if (size == sizeof(device_configuration.interface_config[1]))
            {
                memcpy(pValue, &(device_configuration.interface_config[1]), size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        default:
            retVal = -1;
            break;
    }
    device_config_unlock();

    return retVal;
}

int16_t device_config_set_value(const device_conf_id_t id, uint8_t *pValue, const uint16_t size)
{
    int16_t retVal = 0;
    device_config_lock();
    switch (id)
    {
        case DEVICE_CONF_SETTINGS_PRESENT:
            if (size == sizeof(device_configuration.settings_present))
            {
                memcpy(&(device_configuration.settings_present), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_DHCP_ENABLED:
            if (size == sizeof(device_configuration.dhcp_enabled))
            {
                memcpy(&(device_configuration.dhcp_enabled), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_IP:
            if (size == sizeof(device_configuration.ip))
            {
                memcpy(&(device_configuration.ip), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_NETMASK:
            if (size == sizeof(device_configuration.nm))
            {
                memcpy(&(device_configuration.nm), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_GATEWAY:
            if (size == sizeof(device_configuration.gw))
            {
                memcpy(&(device_configuration.gw), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_MAC:
            if (size == sizeof(device_configuration.mac))
            {
                memcpy(&(device_configuration.mac), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_LEDS_ENABLED:
            if (size == sizeof(device_configuration.leds_enabled))
            {
                memcpy(&(device_configuration.leds_enabled), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_1:
            if (size == sizeof(device_configuration.interface_config[0]))
            {
                memcpy(&(device_configuration.interface_config[0]), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        case DEVICE_CONF_INTERFACE_CONFIG_2:
            if (size == sizeof(device_configuration.interface_config[1]))
            {
                memcpy(&(device_configuration.interface_config[1]), pValue, size);
                retVal = size;
            }
            else
            {
                retVal = -1;
            }
            break;
        default:
            retVal = -1;
            break;
    }
    if (retVal >= 0)
    {
        device_config_write_all_settings();
    }
    device_config_unlock();

    return retVal;
}