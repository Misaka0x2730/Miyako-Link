#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "device_settings.h"
#include "device_config_message.pb.h"
#include "device_config_task.h"

static TaskHandle_t device_config_task;
static StreamBufferHandle_t rx_stream;
static StreamBufferHandle_t tx_stream;
static void device_config_task_thread(void *pParams);

void device_config_task_init(void)
{

    BaseType_t status = xTaskCreate(device_config_task_thread,
                         "mux_task",
                         configMINIMAL_STACK_SIZE*2,
                         NULL,
                         SYSTEM_PRIORITY_LOWEST,
                         &device_config_task);

    vTaskCoreAffinitySet(device_config_task, 0x01);
}

static bool config_message_is_valid(conf_message *message)
{
    conf_message_data_pipe_config *config = message->config;

    for (uint32_t i = 0; i < message->config_count; i++)
    {
        for (uint32_t j = i; j < message->config_count; j++)
        {
            if ((config[i].data_pipe_num == config[j].data_pipe_num) ||
                ((config[i].type == conf_message_pipe_type_ETH) &&
                 (config[j].type == conf_message_pipe_type_ETH) &&
                 (config[i].arg.eth_port == config[j].arg.eth_port)))
            {
                return false;
            }
        }

        if (config->type == conf_message_pipe_type_ETH)
        {
            for (uint32_t j = 0; j < DATA_PIPE_AMOUNT; j++)
            {
                data_pipe_config_t pipe_config;
                const uint32_t settings_offset = DEVICE_SETTINGS_DATA_PIPE_1 - DEVICE_SETTINGS_DATA_PIPE_0;

                device_settings_get_value(DEVICE_SETTINGS_DATA_PIPE_0 + settings_offset * j, (uint8_t*)&pipe_config, sizeof(pipe_config));

                if ((pipe_config.type == conf_message_pipe_type_ETH) && (pipe_config.port == config->arg.eth_port))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

static void device_config_apply_new_config(conf_message *message)
{
    if (config_message_is_valid(message))
    {
        const uint32_t settings_offset = DEVICE_SETTINGS_DATA_PIPE_1 - DEVICE_SETTINGS_DATA_PIPE_0;

        for (uint32_t i = 0; i < message->config_count; i++)
        {
            conf_message_data_pipe_config *pipe_config = &message->config[i];
            const uint32_t pipe_number = pipe_config->data_pipe_num;

            device_setting_id_t setting_id = DEVICE_SETTINGS_DATA_PIPE_0_TYPE + settings_offset * pipe_number;
            device_settings_set_value(setting_id, (uint8_t *) &pipe_config->type, sizeof(data_pipe_type_t), false);

            if (DATA_PIPE_IS_UART(pipe_number))
            {
                setting_id = DEVICE_SETTINGS_DATA_PIPE_0_UART_BAUD + settings_offset * pipe_number;
                device_settings_set_value(setting_id, (uint8_t *) &pipe_config->arg.uart_config.baud,
                                          sizeof(pipe_config->arg.uart_config.baud), false);

                setting_id = DEVICE_SETTINGS_DATA_PIPE_0_UART_BITS_STOP_PARITY + settings_offset * pipe_number;
                const uint8_t bits_parity_stop = pipe_config->arg.uart_config.bits_parity_stop;
                device_settings_set_value(setting_id, (uint8_t *) &bits_parity_stop, sizeof(bits_parity_stop), false);
            }

            if (pipe_config->type == DATA_PIPE_TYPE_ETH)
            {
                setting_id = DEVICE_SETTINGS_DATA_PIPE_0_PORT + settings_offset * pipe_number;
                device_settings_set_value(setting_id, (uint8_t *) &pipe_config->arg.eth_port, sizeof(uint16_t), false);
            }
        }
    }
}

_Noreturn static void device_config_task_thread(void *pParams)
{
    uint8_t message_buf[1024] = { 0 };
    uint32_t message_len = 0;
    uint32_t cur_offset = 0;

    while (1)
    {
        int32_t received_bytes = xStreamBufferReceive(rx_stream, &message_len, sizeof(message_len),
                                                    SYSTEM_WAIT_FOREVER);
        if ((received_bytes == sizeof(message_len)) &&
            (message_len > 0) &&
            (message_len < sizeof(message_buf)))
        {
            cur_offset = 0;

            message_buf[message_len] = '\0';

            uint32_t remaining_size = message_len;

            while (remaining_size)
            {
                received_bytes = xStreamBufferReceive(rx_stream, message_buf + cur_offset, remaining_size,
                                                      pdMS_TO_TICKS(5000));

                if ((received_bytes > 0) && (received_bytes <= remaining_size))
                {
                    remaining_size -= received_bytes;
                    cur_offset += received_bytes;
                }
                else
                {
                    break;
                }
            }

            if (remaining_size == 0)
            {
                conf_message config_message = conf_message_init_zero;

                pb_istream_t in_stream = pb_istream_from_buffer(message_buf, message_len);

                /* Now we are ready to decode the message. */
                bool pb_status = pb_decode(&in_stream, conf_message_fields, &config_message);

                if (pb_status)
                {
                    bool settings_changed = false;

                    if (config_message.type == conf_message_message_type_WRITE)
                    {
                        settings_changed = true;
                        device_config_apply_new_config(&config_message);
                    }
                    else
                    {
                        conf_message response = conf_message_init_zero;
                        pb_ostream_t out_stream = pb_ostream_from_buffer(message_buf, sizeof(message_buf));

                    }

                    if (settings_changed)
                    {
                        device_settings_save_settings();
                    }
                }
            }

        }
    }
}