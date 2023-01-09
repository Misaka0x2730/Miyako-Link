#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#define DEFAULT_GDB_PORT                (10032)
#define DEFAULT_TELNET_PORT             (10033)

#define DEFAULT_TELNET_UART_BAUD        (115200)
#define DEFAULT_TELNET_BITS_PARITY_STOP (0)

typedef enum
{
    DATA_PIPE_FIRST = 0,
    DATA_PIPE_TARGET1 = DATA_PIPE_FIRST,
    DATA_PIPE_TARGET1_UART,
    DATA_PIPE_TARGET2,
    DATA_PIPE_TARGET2_UART,
    DATA_PIPE_LAST = DATA_PIPE_TARGET2_UART,
    DATA_PIPE_COUNT
} data_pipe_t;

#define DATA_PIPE_IS_TARGET(x)    (((x) == DATA_PIPE_TARGET1)      || ((x) == DATA_PIPE_TARGET2))
#define DATA_PIPE_IS_UART(x)      (((x) == DATA_PIPE_TARGET1_UART) || ((x) == DATA_PIPE_TARGET2_UART))

typedef enum
{
    INTERFACE_TYPE_FIRST = 0,
    INTERFACE_TYPE_USB = INTERFACE_TYPE_FIRST,
    INTERFACE_TYPE_ETH,
    INTERFACE_TYPE_LAST = INTERFACE_TYPE_ETH
} host_interface_type_t;

typedef enum
{
    DATA_PIPE_TYPE_FIRST = 0,
    DATA_PIPE_TYPE_DISABLED = DATA_PIPE_TYPE_FIRST,
    DATA_PIPE_TYPE_USB,
    DATA_PIPE_TYPE_ETH,
    DATA_PIPE_TYPE_LAST = DATA_PIPE_TYPE_ETH
} data_pipe_type_t;

typedef struct
{
    host_interface_type_t interface_type;
    uint16_t port;
} __attribute__((packed)) host_interface_conf_t;

typedef struct
{
    uint32_t frequency;
} __attribute__((packed)) target_interface_conf_t;

typedef struct
{
    uint32_t baud;
    uint8_t bits_parity_stop;
} __attribute__((packed)) uart_config_t;

typedef struct
{
    data_pipe_type_t type;

    union
    {
        uint16_t port;
        uart_config_t uart_config;
    };
} __attribute__((packed)) data_pipe_config_t;

typedef struct
{
    uint16_t crc; // Should be always at the beginning
    bool dhcp_enabled;
    uint8_t ip[4];
    uint8_t nm[4];
    uint8_t gw[4];
    uint32_t leds_enabled;

    data_pipe_config_t data_pipe[DATA_PIPE_COUNT];

    uint8_t mac[6]; // Should be always at the end
} __attribute__((packed)) device_settings_t;

typedef enum
{
    DEVICE_SETTINGS_FIRST = 0,
    DEVICE_SETTINGS_CRC = DEVICE_SETTINGS_FIRST,
    DEVICE_SETTINGS_DHCP_ENABLED,
    DEVICE_SETTINGS_IP,
    DEVICE_SETTINGS_NETMASK,
    DEVICE_SETTINGS_GATEWAY,
    DEVICE_SETTINGS_MAC,
    DEVICE_SETTINGS_LEDS_ENABLED,

    DEVICE_SETTINGS_DATA_PIPE_0,
    DEVICE_SETTINGS_DATA_PIPE_0_TYPE,
    DEVICE_SETTINGS_DATA_PIPE_0_PORT,
    DEVICE_SETTINGS_DATA_PIPE_0_UART_CONFIG,
    DEVICE_SETTINGS_DATA_PIPE_0_UART_BAUD,
    DEVICE_SETTINGS_DATA_PIPE_0_UART_BITS_STOP_PARITY,

    DEVICE_SETTINGS_DATA_PIPE_1,
    DEVICE_SETTINGS_DATA_PIPE_1_TYPE,
    DEVICE_SETTINGS_DATA_PIPE_1_PORT,
    DEVICE_SETTINGS_DATA_PIPE_1_UART_CONFIG,
    DEVICE_SETTINGS_DATA_PIPE_1_UART_BAUD,
    DEVICE_SETTINGS_DATA_PIPE_1_UART_BITS_STOP_PARITY,

    DEVICE_SETTINGS_DATA_PIPE_2,
    DEVICE_SETTINGS_DATA_PIPE_2_TYPE,
    DEVICE_SETTINGS_DATA_PIPE_2_PORT,
    DEVICE_SETTINGS_DATA_PIPE_2_UART_CONFIG,
    DEVICE_SETTINGS_DATA_PIPE_2_UART_BAUD,
    DEVICE_SETTINGS_DATA_PIPE_2_UART_BITS_STOP_PARITY,

    DEVICE_SETTINGS_DATA_PIPE_3,
    DEVICE_SETTINGS_DATA_PIPE_3_TYPE,
    DEVICE_SETTINGS_DATA_PIPE_3_PORT,
    DEVICE_SETTINGS_DATA_PIPE_3_UART_CONFIG,
    DEVICE_SETTINGS_DATA_PIPE_3_UART_BAUD,
    DEVICE_SETTINGS_DATA_PIPE_3_UART_BITS_STOP_PARITY,

    DEVICE_CONF_LAST = DEVICE_SETTINGS_DATA_PIPE_3_UART_CONFIG
} device_setting_id_t;

void device_settings_init(void);
int32_t device_settings_get_value(const device_setting_id_t id, uint8_t *pValue, const size_t size);
int32_t device_settings_set_value(const device_setting_id_t id, uint8_t *pValue, const size_t size, const bool save_to_eeprom);
void device_settings_save_settings(void);

#endif // __DEVICE_CONFIG_H