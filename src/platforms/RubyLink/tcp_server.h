#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include "general.h"
#include "w5500_common.h"

typedef struct
{
    uint8_t socket_number;
    uint16_t port;
    bool send_eot;
    StreamBufferHandle_t rx_stream;
    StreamBufferHandle_t tx_stream;
} tcp_server_config_t;

void tcp_server_add(tcp_server_config_t *p_tcp_server_config);

#endif // _TCP_SERVER_H
