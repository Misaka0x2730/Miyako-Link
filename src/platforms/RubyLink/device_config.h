#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#define SETTINGS_PRESENT    (0x2730)

typedef enum
{
    INTERFACE_TYPE_USB = 0,
    INTERFACE_TYPE_ETH,
    INTERFACE_TYPE_INVALID
} interface_type_t;

typedef struct
{
    uint8_t interface_type;
    uint16_t port;
} __attribute__((packed)) interface_conf_t;

typedef struct
{
    uint32_t frequency;
} __attribute__((packed)) target_interface_conf_t;

typedef struct
{
    uint16_t crc; // Should be always at the beginning
    uint8_t dhcp_enabled;
    uint8_t ip[4];
    uint8_t nm[4];
    uint8_t gw[4];
    uint8_t leds_enabled[4];

    interface_conf_t interface_config[2];
    target_interface_conf_t target_interface_conf[2];

    uint8_t mac[6]; // Should be always at the end
} __attribute__((packed)) device_conf_t;

typedef enum
{
    DEVICE_CONF_CRC = 0,
    DEVICE_CONF_DHCP_ENABLED,
    DEVICE_CONF_IP,
    DEVICE_CONF_NETMASK,
    DEVICE_CONF_GATEWAY,
    DEVICE_CONF_MAC,
    DEVICE_CONF_LEDS_ENABLED,
    DEVICE_CONF_INTERFACE_CONFIG_1,
    DEVICE_CONF_INTERFACE_CONFIG_1_TYPE,
    DEVICE_CONF_INTERFACE_CONFIG_1_PORT,
    DEVICE_CONF_INTERFACE_CONFIG_2,
    DEVICE_CONF_INTERFACE_CONFIG_2_TYPE,
    DEVICE_CONF_INTERFACE_CONFIG_2_PORT,
    DEVICE_CONF_TARGET_INTERFACE_CONFIG_1,
    DEVICE_CONF_TARGET_INTERFACE_CONFIG_1_FREQ,
    DEVICE_CONF_TARGET_INTERFACE_CONFIG_2,
    DEVICE_CONF_TARGET_INTERFACE_CONFIG_2_FREQ,
    DEVICE_CONF_MAX_ID = DEVICE_CONF_TARGET_INTERFACE_CONFIG_2_FREQ
} device_conf_id_t;

void device_config_init(void);
void device_config_reset_to_default(void);
int32_t device_config_read_all_settings(void);
int32_t device_config_write_all_settings(void);
int32_t device_config_get_value(const device_conf_id_t id, uint8_t *pValue, const size_t size);
int32_t device_config_set_value(const device_conf_id_t id, uint8_t *pValue, const size_t size);

#endif // __DEVICE_CONFIG_H