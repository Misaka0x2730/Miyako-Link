#include "w5500_common.h"
#include "w5500_socket.h"

#include "wizchip_conf.h"

static uint16_t socket_tx_addr[W5500_SOCK_AMOUNT] = { 0 };
static uint16_t socket_rx_addr[W5500_SOCK_AMOUNT] = { 0 };

void w5500_reset_socket_state(const uint8_t socket_number)
{
    if (socket_number >= W5500_SOCK_AMOUNT)
    {
        return;
    }

    uint8_t reset_pointer[sizeof(uint16_t)] = { 0 };

    socket_tx_addr[socket_number] = 0;
    socket_rx_addr[socket_number] = 0;

    WIZCHIP_WRITE_BUF(Sn_RX_WR(socket_number),reset_pointer, sizeof(reset_pointer));
    WIZCHIP_WRITE_BUF(Sn_TX_WR(socket_number),reset_pointer, sizeof(reset_pointer));
}

void w5500_set_socket_irq_mask(const uint8_t socket_number, const w5500_sock_irq_t irq_mask)
{
    setSn_IMR(socket_number, irq_mask);
}

w5500_sock_irq_t w5500_get_socket_irq_status(const uint8_t socket_number)
{
    if (socket_number >= W5500_SOCK_AMOUNT)
    {
        return 0;
    }

    return getSn_IR(socket_number);
}

void w5500_clear_socket_all_irq_status(const uint8_t socket_number)
{
    if (socket_number >= W5500_SOCK_AMOUNT)
    {
        return;
    }

    setSn_IR(socket_number, W5500_SOCK_IRQ_ALL);
}

w5500_sock_state_t w5500_get_socket_status(const uint8_t socket_number)
{
    return (w5500_sock_state_t)getSn_SR(socket_number);
}

size_t w5500_read(const uint8_t socket_number, uint8_t *pData, const size_t len)
{
    if(len == 0)
    {
        return 0;
    }

    /* Waiting for previous operation is finished */
    while(w5500_read_byte_register(Sn_CR(socket_number)));

    /* Read Sn_RX_RSR register (data amount in socket buffer */
    uint8_t reg[sizeof(uint16_t)] = { 0 };

    size_t bytes_to_read = w5500_read_two_byte_register(Sn_RX_RSR(socket_number));

    if (bytes_to_read == 0)
    {
        return 0;
    }

    if (bytes_to_read > len)
    {
        bytes_to_read = len;
    }

    uint16_t read_pointer = w5500_read_two_byte_register(Sn_RX_RD(socket_number));
    const uint32_t addrsel = ((uint32_t)read_pointer << 8) + (WIZCHIP_RXBUF_BLOCK(socket_number) << 3);

    WIZCHIP_READ_BUF(addrsel, pData, bytes_to_read);
    socket_rx_addr[socket_number] += bytes_to_read;

    read_pointer += bytes_to_read;
    w5500_write_two_byte_register(Sn_RX_RD(socket_number), read_pointer);

    w5500_write_byte_register(Sn_CR(socket_number), Sn_CR_RECV);

    return bytes_to_read;
}

size_t w5500_send(const uint8_t socket_number, uint8_t *pData, const size_t len)
{
    if(len == 0)
    {
        return 0;
    }

    /* Waiting for previous operation is finished */
    while(w5500_read_byte_register(Sn_CR(socket_number)));

    uint8_t reg[sizeof(uint16_t)] = { 0 };

    size_t free_space = w5500_read_two_byte_register(Sn_TX_FSR(socket_number));
    size_t bytes_to_write = len;

    if (free_space == 0)
    {
        return 0;
    }

    if (bytes_to_write > free_space)
    {
        bytes_to_write = free_space;
    }

    uint16_t write_pointer = w5500_read_two_byte_register(Sn_TX_WR(socket_number));
    const uint32_t addrsel = ((uint32_t)write_pointer << 8) + (WIZCHIP_TXBUF_BLOCK(socket_number) << 3);

    WIZCHIP_WRITE_BUF(addrsel,pData, bytes_to_write);
    write_pointer += bytes_to_write;

    w5500_write_two_byte_register(Sn_TX_WR(socket_number), write_pointer);
    socket_tx_addr[socket_number] += bytes_to_write;

    w5500_write_byte_register(Sn_CR(socket_number), Sn_CR_SEND);

    return bytes_to_write;
}