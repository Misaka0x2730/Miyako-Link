#include "general.h"
#include "FreeRTOS.h"
#include "w5500_common.h"
#include "w5500_socket.h"
#include "w5500_task.h"
#include "stream_buffer.h"
#include "socket.h"
#include "tcp_server.h"

void tcp_server_rx_task_thread(void* params);
void tcp_server_tx_task_thread(void* params);

void tcp_server_add(tcp_server_config_t *p_tcp_server_config)
{
    char task_name[configMAX_TASK_NAME_LEN] = { 0 };

    if (p_tcp_server_config->socket_number == W5500_SOCK_UNUSED)
    {
        p_tcp_server_config->socket_number = w5500_find_unused_socket();

        if (p_tcp_server_config->socket_number == W5500_SOCK_UNUSED)
        {
            //TODO: assert
        }
    }

    TaskHandle_t rx_task;
    snprintf(task_name, sizeof(task_name), "tcp_server_%d_rx_thread_port_%d", p_tcp_server_config->socket_number, p_tcp_server_config->port);
    BaseType_t status = xTaskCreate(tcp_server_rx_task_thread,
                                    task_name,
                                    configMINIMAL_STACK_SIZE*8,
                                    p_tcp_server_config,
                                    SYSTEM_PRIORITY_LOWEST,
                                    &rx_task);

    if (status != pdTRUE)
    {
        //TODO: assert
    }

    vTaskCoreAffinitySet(rx_task, 0x01);

    TaskHandle_t tx_task;
    snprintf(task_name, sizeof(task_name), "tcp_server_%d_tx_thread_port_%d", p_tcp_server_config->socket_number, p_tcp_server_config->port);
    status = xTaskCreate(tcp_server_tx_task_thread,
                         task_name,
                         configMINIMAL_STACK_SIZE*8,
                         p_tcp_server_config,
                         SYSTEM_PRIORITY_LOWEST,
                         &tx_task);

    if (status != pdTRUE)
    {
        //TODO: assert
    }

    vTaskCoreAffinitySet(tx_task, 0x01);

    w5500_tcp_server_register_socket_tasks(p_tcp_server_config->socket_number, rx_task, tx_task);
}

_Noreturn void tcp_server_rx_task_thread(void* params)
{
    tcp_server_config_t *config = (tcp_server_config_t*)(params);
    const uint32_t sock = config->socket_number;
    TaskHandle_t w5500_task = w5500_get_task_handle();

    if (sock >= W5500_SOCK_AMOUNT)
    {
        //TODO: assert
    }

    w5500_lock();

    w5500_set_socket_irq_mask(sock, W5500_SOCK_IRQ_ALL);

    const uint8_t current_interrupt = w5500_get_global_sockets_irq_mask();
    w5500_set_global_sockets_irq_mask(current_interrupt | (1 << sock));

    w5500_unlock();

    uint8_t rx_buf[4096] = { 0 };
    uint32_t notificationValue = 0;

    while (1)
    {
        w5500_lock();

        w5500_sock_state_t socket_status = w5500_get_socket_status(sock);

        switch(socket_status)
        {
            case W5500_SOCK_STATE_CLOSE_WAIT:
                disconnect(sock);
            case W5500_SOCK_STATE_CLOSED:
            {
                if (config->send_eot)
                {
                    const char data_to_send = 0x04; // Send EOT
                    xStreamBufferSend(config->rx_stream, &data_to_send, sizeof(data_to_send),
                                      SYSTEM_WAIT_DONT_BLOCK);
                }

                socket(sock, Sn_MR_TCP, config->port, SF_IO_NONBLOCK);
                listen(sock);
                break;
            }
            case W5500_SOCK_STATE_ESTABLISHED:
            {
                uint8_t clear_send_interrupt = SIK_CONNECTED;
                ctlsocket(sock, CS_CLR_INTERRUPT, &clear_send_interrupt);

                int32_t bytes_in_rx_buf = getSn_RX_RSR(sock);

                clear_send_interrupt = SIK_RECEIVED;
                ctlsocket(sock, CS_CLR_INTERRUPT, &clear_send_interrupt);

                if (bytes_in_rx_buf > 0)
                {
                    do
                    {
                        while (bytes_in_rx_buf > 0)
                        {
                            const uint32_t bytes_to_read = (bytes_in_rx_buf > sizeof(rx_buf)) ? sizeof(rx_buf)
                                                                                              : bytes_in_rx_buf;
                            const size_t received_bytes = w5500_read(sock, rx_buf, bytes_to_read);

                            if (received_bytes > 0)
                            {
                                bytes_in_rx_buf -= received_bytes;
                                if (xStreamBufferSend(config->rx_stream, rx_buf, received_bytes,
                                                      pdMS_TO_TICKS(200)) != received_bytes)
                                {
                                    break;
                                    //TODO: looks like the rx queue is full
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                        bytes_in_rx_buf = getSn_RX_RSR(sock);
                    }
                    while (bytes_in_rx_buf > 0);
                }
                break;
            }
            default:
                break;
        }

        w5500_unlock();

        xTaskNotify(w5500_task, W5500_TASK_RX_ACK(sock), eSetBits);
        xTaskNotifyWait(0, W5500_TASK_INTERRUPT_OCCURRED, &notificationValue, SYSTEM_WAIT_FOREVER);

        /*const int32_t received_bytes = recv(sock, rx_buf, sizeof(rx_buf));

        if (received_bytes > 0)
        {
            if (xStreamBufferSend(config->rx_stream, rx_buf, received_bytes,
                                  pdMS_TO_TICKS(200)) != received_bytes)
            {
                //TODO: looks like the rx queue is full
            }
        }*/
    }
}

_Noreturn void tcp_server_tx_task_thread(void* params)
{
    tcp_server_config_t *config = (tcp_server_config_t*)(params);
    const uint32_t sock = config->socket_number;

    if (sock >= _WIZCHIP_SOCK_NUM_)
    {
        //TODO: assert
    }

    uint16_t data = 0;
    uint16_t data_length = 0;
    uint8_t tx_buf[512] = { 0 };
    uint32_t notificationValue = 0;
    TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
    while (1)
    {
        int32_t bytes_to_send = xStreamBufferReceive(config->tx_stream, tx_buf, sizeof(tx_buf),
                                                     pdMS_TO_TICKS(10));

        if (bytes_to_send > 0)
        {
            int32_t cur_offset = 0;
            uint8_t send_attempts_count = 0;
            do
            {
                xTaskNotifyStateClear(currentTaskHandle);

                w5500_lock();

                const w5500_sock_state_t socket_status = w5500_get_socket_status(sock);

                if (socket_status != W5500_SOCK_STATE_ESTABLISHED)
                {
                    w5500_unlock();
                    break;
                }

                const size_t sent_bytes = w5500_send(sock, tx_buf + cur_offset, bytes_to_send);
                w5500_unlock();

                if (sent_bytes == 0)
                {
                    if (++send_attempts_count >= W5500_TX_MAX_ATTEMPT_COUNT)
                    {
                        break;
                    }

                    xTaskNotifyWait(0, W5500_TX_RX_TASK_SEND_OK | W5500_TX_RX_TASK_TIMEOUT, &notificationValue,
                                    pdMS_TO_TICKS(200));

                    if (notificationValue & W5500_TX_RX_TASK_TIMEOUT)
                    {
                        break;
                    }
                }
                else
                {
                    send_attempts_count = 0;
                    bytes_to_send -= sent_bytes;
                    cur_offset += sent_bytes;
                    if (bytes_to_send <= 0)
                    {
                        //uint8_t clear_send_interrupt = SIK_SENT;

                        /*w5500_lock();
                        ctlsocket(sock, CS_CLR_INTERRUPT, &clear_send_interrupt);
                        w5500_unlock();*/
                    }
                    //xTaskNotify(w5500_task, W5500_TASK_TX_ACK(sock), eSetBits);
                }
            }
            while (bytes_to_send > 0);
        }
    }
}