#ifndef _W5500_TASK_H
#define _W5500_TASK_H

#include "w5500_common.h"
#include "w5500_socket.h"

#define W5500_TASK_INTERRUPT_OCCURRED  (0x01)
#define W5500_TASK_RX_ACK(sn)          (0x02 << (sn)*2)
#define W5500_TASK_TX_ACK(sn)          (0x04 << (sn)*2)

#define W5500_TX_RX_TASK_SEND_OK       (W5500_SOCK_IRQ_SENDOK)
#define W5500_TX_RX_TASK_TIMEOUT       (W5500_SOCK_IRQ_TIMEOUT)
#define W5500_TX_RX_TASK_DATA_RECV     (W5500_SOCK_IRQ_RECV)

void w5500_lock(void);
void w5500_unlock(void);

uint8_t w5500_find_unused_socket(void);

TaskHandle_t w5500_get_task_handle(void);
void w5500_task_init(void);
void w5500_tcp_server_register_socket_tasks(uint8_t socket_number, TaskHandle_t rx_task, TaskHandle_t tx_task);

#endif //_W5500_TASK_H
