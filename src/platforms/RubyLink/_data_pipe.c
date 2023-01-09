#include "pb_encode.h"
#include "pb_decode.h"
#include "device_settings.h"
#include "device_config_message.pb.h"
#include "system_func.h"
#include "data_pipe.h"

static data_pipe_config_t data_pipes_config[DATA_PIPE_COUNT] = { 0 };

static bool data_pipe_is_allowed_pair(data_pipe_config_t *p_pipe_a, data_pipe_config_t *p_pipe_b)
{
    /* Two pipes to one debug interface is not allowed... yet */
    if ((p_pipe_a->target_side.type == DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD) &&
        (p_pipe_b->target_side.type == DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD) &&
        (p_pipe_a->target_side.debug_config.interface == p_pipe_b->target_side.debug_config.interface))
    {
        return false;
    }

    /* Two pipes to one port is not allowed for debug interfaces */
    if (((p_pipe_a->target_side.type == DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD) ||
         (p_pipe_b->target_side.type == DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD)) &&
         (p_pipe_a->host_side.type == DATA_PIPE_HOST_SIDE_TYPE_ETH) &&
         (p_pipe_b->host_side.type == DATA_PIPE_HOST_SIDE_TYPE_ETH) &&
         (p_pipe_a->host_side.eth_config.port == p_pipe_b->host_side.eth_config.port))
    {
        return false;
    }

    return true;
}

static bool data_pipe_check_config(data_pipe_config_t *p_data)
{
    for (uint32_t i = 0; i < (DATA_PIPE_COUNT - 1); i++)
    {
        for (uint32_t j = i + 1; j < DATA_PIPE_COUNT; j++)
        {
            if (data_pipe_is_allowed_pair(&p_data[i], &p_data[j]) == false)
            {
                return false;
            }
        }
    }

    return true;
}

void data_pipe_init(void)
{
    SYS_ASSERT(device_settings_initialized() == true);

    data_pipe_config_t all_pipes_config[DATA_PIPE_COUNT] = { 0 };

    for (uint32_t i = 0; i < DATA_PIPE_COUNT; i++)
    {
        const uint32_t offset = DEVICE_SETTINGS_DATA_PIPE_1 - DEVICE_SETTINGS_DATA_PIPE_0;
        const device_setting_id_t setting_id = DEVICE_SETTINGS_DATA_PIPE_0 + (offset * i);
        const size_t size = sizeof(data_pipe_config_t);
        if (device_settings_get_value(setting_id, (uint8_t*)(&all_pipes_config[i]), size) != size)
        {
            //TODO: add something
        }
    }

    if (data_pipe_check_config(all_pipes_config) == false)
    {
        data_pipe_set_default_config_for_all_pipes(all_pipes_config);

        for (uint32_t i = 0; i < DATA_PIPE_COUNT; i++)
        {
            const uint32_t offset = DEVICE_SETTINGS_DATA_PIPE_1 - DEVICE_SETTINGS_DATA_PIPE_0;
            const device_setting_id_t setting_id = DEVICE_SETTINGS_DATA_PIPE_0 + (offset * i);
            const size_t size = sizeof(data_pipe_config_t);
            if (device_settings_set_value(setting_id, (uint8_t*)(&all_pipes_config[i]), size, (i == (DATA_PIPE_COUNT - 1)) ? true : false) != size)
            {
                //TODO: add something
            }
        }
    }
}

void data_pipe_set_default_config_for_all_pipes(data_pipe_config_t *p_data)
{
    /* First data pipe: ETH (port 10032) <-> debug interface 1 (non-isolated) */
    p_data[DATA_PIPE_0].pipe = DATA_PIPE_0;
    p_data[DATA_PIPE_0].state = DATA_PIPE_STATE_ENABLED;
    p_data[DATA_PIPE_0].host_side.type = DATA_PIPE_HOST_SIDE_TYPE_ETH;
    p_data[DATA_PIPE_0].host_side.eth_config.port = DEFAULT_GDB_PORT;

    p_data[DATA_PIPE_0].target_side.type = DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD;
    p_data[DATA_PIPE_0].target_side.debug_config.interface = DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_1;
    p_data[DATA_PIPE_0].target_side.debug_config.frequency = DEFAULT_JTAG_SWD_FREQ;

    /* Second data pipe: ETH (port 10033) <-> serial interface 1 (non-isolated) */
    p_data[DATA_PIPE_1].pipe = DATA_PIPE_1;
    p_data[DATA_PIPE_1].state = DATA_PIPE_STATE_ENABLED;
    p_data[DATA_PIPE_1].host_side.type = DATA_PIPE_HOST_SIDE_TYPE_ETH;
    p_data[DATA_PIPE_1].host_side.eth_config.port = DEFAULT_TELNET_PORT;

    p_data[DATA_PIPE_1].target_side.type = DATA_PIPE_TARGET_SIDE_TYPE_SERIAL;
    p_data[DATA_PIPE_1].target_side.serial_config.interface = DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_1;
    p_data[DATA_PIPE_1].target_side.serial_config.baud = DEFAULT_TELNET_UART_BAUD;
    p_data[DATA_PIPE_1].target_side.serial_config.bits_parity_stop = DEFAULT_TELNET_BITS_PARITY_STOP;

    /* Third data pipe: USB <-> debug interface 2 (isolated) */
    p_data[DATA_PIPE_2].pipe = DATA_PIPE_2;
    p_data[DATA_PIPE_2].state = DATA_PIPE_STATE_ENABLED;
    p_data[DATA_PIPE_2].host_side.type = DATA_PIPE_HOST_SIDE_TYPE_USB;

    p_data[DATA_PIPE_2].target_side.type = DATA_PIPE_TARGET_SIDE_TYPE_JTAG_SWD;
    p_data[DATA_PIPE_2].target_side.debug_config.interface = DATA_PIPE_TARGET_SIDE_DEBUG_INTERFACE_2;
    p_data[DATA_PIPE_2].target_side.debug_config.frequency = DEFAULT_JTAG_SWD_FREQ;

    /* Fourth data pipe: USB <-> serial interface 2 (isolated) */
    p_data[DATA_PIPE_3].pipe = DATA_PIPE_3;
    p_data[DATA_PIPE_3].state = DATA_PIPE_STATE_ENABLED;
    p_data[DATA_PIPE_3].host_side.type = DATA_PIPE_HOST_SIDE_TYPE_USB;

    p_data[DATA_PIPE_3].target_side.type = DATA_PIPE_TARGET_SIDE_TYPE_SERIAL;
    p_data[DATA_PIPE_3].target_side.serial_config.interface = DATA_PIPE_TARGET_SIDE_SERIAL_INTERFACE_2;
    p_data[DATA_PIPE_3].target_side.serial_config.baud = DEFAULT_TELNET_UART_BAUD;
    p_data[DATA_PIPE_3].target_side.serial_config.bits_parity_stop = DEFAULT_TELNET_BITS_PARITY_STOP;
}

