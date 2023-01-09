#ifndef _W5500_SOCKET_H
#define _W5500_SOCKET_H

#include "general.h"

typedef enum
{
    W5500_SOCK_STATE_CLOSED      = 0x00,
    W5500_SOCK_STATE_ESTABLISHED = 0x17,
    W5500_SOCK_STATE_CLOSE_WAIT  = 0x1C
} w5500_sock_state_t;

typedef enum
{
    W5500_SOCK_IRQ_CON      = 0x01,
    W5500_SOCK_IRQ_DISCON   = 0x02,
    W5500_SOCK_IRQ_RECV     = 0x04,
    W5500_SOCK_IRQ_TIMEOUT  = 0x08,
    W5500_SOCK_IRQ_SENDOK   = 0x10,
    W5500_SOCK_IRQ_ALL      = 0x1F
} w5500_sock_irq_t;

void w5500_reset_socket_state(const uint8_t socket_number);
void w5500_set_socket_irq_mask(const uint8_t socket_number, const w5500_sock_irq_t irq_mask);
w5500_sock_irq_t w5500_get_socket_irq_status(const uint8_t socket_number);
void w5500_clear_socket_all_irq_status(const uint8_t socket_number);
w5500_sock_state_t w5500_get_socket_status(const uint8_t socket_number);
size_t w5500_read(const uint8_t socket_number, uint8_t *pData, const size_t len);
size_t w5500_send(const uint8_t socket_number, uint8_t *pData, const size_t len);

#endif //_W5500_SOCKET_H
