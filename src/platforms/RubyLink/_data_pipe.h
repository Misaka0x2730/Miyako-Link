#ifndef MIYAKOLINK_DATA_PIPE_H
#define MIYAKOLINK_DATA_PIPE_H

#include <stdint.h>

#define DATA_PIPE_COUNT     (4)

typedef enum
{
    DATA_PIPE_FIRST = 0,
    DATA_PIPE_0 = DATA_PIPE_FIRST,
    DATA_PIPE_1,
    DATA_PIPE_2,
    DATA_PIPE_3,
    DATA_PIPE_LAST = DATA_PIPE_3
} data_pipe_t;

#if (DATA_PIPE_COUNT != (DATA_PIPE_LAST + 1))
#error "DATA_PIPE_COUNT should be (DATA_PIPE_LAST + 1). Fix data_pipe_t enum or DATA_PIPE_COUNT define"
#endif

typedef enum
{
    DATA_PIPE_HOST_SIDE_TYPE_FIRST = 0,
    DATA_PIPE_HOST_SIDE_TYPE_USB = DATA_PIPE_HOST_SIDE_TYPE_FIRST,
    DATA_PIPE_HOST_SIDE_TYPE_ETH,
    DATA_PIPE_HOST_SIDE_TYPE_LAST = DATA_PIPE_HOST_SIDE_TYPE_ETH
} data_pipe_host_side_type_t;

typedef struct
{
    uint16_t port;
} __attribute__((packed)) data_pipe_host_side_eth_config_t;

typedef struct
{
    data_pipe_host_side_type_t type;
    data_pipe_host_side_eth_config_t eth_config;
} __attribute__((packed)) data_pipe_host_side_t;

typedef enum
{
    DATA_PIPE_TARGET_SIDE_TYPE_FIRST = 0,
    DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD = DATA_PIPE_TARGET_SIDE_TYPE_FIRST,
    DATA_PIPE_TARGET_SIDE_TYPE_SERIAL,
    DATA_PIPE_HOST_INTERFACE_TYPE_LAST = DATA_PIPE_TARGET_SIDE_TYPE_SERIAL
} data_pipe_target_side_type_t;

/* Multiuple connections (two pipes to one target side) are not allowed for SWD/JTAG yet */
#define DATA_PIPE_TARGET_SIDE_IS_ALLOWED_MULTI(x)   ((x) != DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD)

typedef enum
{
    DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_FIRST = 0,
    DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_1 = DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_FIRST, /* non-isolated interface */
    DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_2, /* isolated interface */
    DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_LAST = DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_2,
    DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_COUNT
} data_pipe_target_side_serial_interface_t;

typedef struct
{
    data_pipe_target_side_serial_interface_t interface;
    uint32_t baud;
    uint8_t bits_parity_stop;
} __attribute__((packed)) data_pipe_target_side_serial_config_t;

typedef enum
{
    DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_FIRST = 0,
    DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_1 = DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_FIRST, /* non-isolated interface */
    DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_2, /* isolated interface */
    DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_LAST = DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_2,
    DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_COUNT
} data_pipe_target_side_debug_interface_t;

typedef struct
{
    data_pipe_target_side_debug_interface_t interface;
    uint32_t frequency;
} __attribute__((packed)) data_pipe_target_side_debug_config_t;

typedef struct
{
    data_pipe_target_side_type_t type;
    union
    {
        data_pipe_target_side_serial_config_t serial_config;
        data_pipe_target_side_debug_config_t debug_config;
    };
} __attribute__((packed)) data_pipe_target_side_t;

typedef enum
{
    DATA_PIPE_STATE_DISABLED = 0,
    DATA_PIPE_STATE_ENABLED
} data_pipe_state_t;

typedef struct
{
    data_pipe_t pipe;
    data_pipe_state_t state;
    data_pipe_host_side_t host_side;
    data_pipe_target_side_t target_side;
} __attribute__((packed)) data_pipe_config_t;

void data_pipe_set_default_config_for_all_pipes(data_pipe_config_t *p_data);

#endif //MIYAKOLINK_DATA_PIPE_H
