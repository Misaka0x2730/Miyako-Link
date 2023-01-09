#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "hardware/dma.h"
#include "system_pins.h"
#include "general.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "semphr.h"
#include "task.h"
#include "system_func.h"
#include "mem_manager.h"
#include "socket.h"
#include "device_settings.h"
#include "w5500_task.h"
#include "w5500_common.h"
#include "w5500_socket.h"

uint32_t dma_rx_channel = 0;
uint32_t dma_tx_channel = 0;
dma_channel_config dma_rx_config;
dma_channel_config dma_tx_config;

void tcp_server_rx_task_thread(void* params);
void tcp_server_tx_task_thread(void* params);

typedef struct
{
    TaskHandle_t rx_task;
    TaskHandle_t tx_task;
} socket_tasks_t;

static socket_tasks_t w5500_socket_task_list[W5500_SOCK_AMOUNT] = { 0 };
static uint8_t w5500_socket_list[W5500_SOCK_AMOUNT];

static SemaphoreHandle_t w5500_mutex;
static TaskHandle_t w5500_task;

void w5500_int_callback(uint gpio, uint32_t events)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    traceISR_ENTER();
    xTaskNotifyFromISR(w5500_task, W5500_TASK_INTERRUPT_OCCURRED, eSetBits, &xHigherPriorityTaskWoken);
    irq_set_enabled(IO_IRQ_BANK0, false);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void w5500_select(void)
{
    gpio_put(PIN_ETH_CS, 0);
}

void w5500_deselect(void)
{
    gpio_put(PIN_ETH_CS, 1);
}

const uint8_t dummy_data = 0xFF;
static uint8_t w5500_write_buf[4096];
static uint8_t w5500_read_buf[4096];

void w5500_read_burst(uint8_t* pBuf, uint16_t len)
{
    memset(w5500_write_buf, 0xFF, sizeof(w5500_write_buf));

    dma_channel_configure(dma_rx_channel, &dma_rx_config,
                          w5500_read_buf, // write address
                          &spi_get_hw(spi0)->dr, // read address
                          len, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    dma_channel_configure(dma_tx_channel, &dma_tx_config,
                          &spi_get_hw(spi0)->dr, // write address
                          w5500_write_buf, // read address
                          len, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    hw_write_masked(&dma_channel_hw_addr(dma_rx_channel)->ctrl_trig,
                    1 << DMA_CH0_CTRL_TRIG_HIGH_PRIORITY_MSB,
                    DMA_CH0_CTRL_TRIG_HIGH_PRIORITY_BITS);

    dma_start_channel_mask((1u << dma_tx_channel) | (1u << dma_rx_channel));

    dma_channel_wait_for_finish_blocking(dma_rx_channel);

    /*hw_write_masked(&dma_channel_hw_addr(dma_tx_channel)->ctrl_trig,
                    0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                    DMA_CH0_CTRL_TRIG_EN_BITS);

    hw_write_masked(&dma_channel_hw_addr(dma_rx_channel)->ctrl_trig,
                    0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                    DMA_CH0_CTRL_TRIG_EN_BITS);*/

    //channel_config_set_read_increment(&dma_tx_config, true);

    //busy_wait_ms(2);
    memcpy(pBuf, w5500_read_buf, len);
    //spi_read_blocking(spi0, 0xFF, pBuf, len);
}

uint8_t w5500_write_burst(uint8_t* pBuf, uint16_t len)
{
    uint8_t ret = 0;
    memcpy(w5500_write_buf, pBuf, len);

    //channel_config_set_write_increment(&dma_rx_config, false);

    dma_channel_configure(dma_tx_channel, &dma_tx_config,
                          &spi_get_hw(spi0)->dr, // write address
                          w5500_write_buf, // read address
                          len, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    /*dma_channel_configure(dma_rx_channel, &dma_rx_config,
                          w5500_read_buf, // write address
                          &spi_get_hw(spi0)->dr, // read address
                          len, // element count (each element is of size transfer_data_size)
                          false); // don't start yet*/

    dma_start_channel_mask((1u << dma_tx_channel));

    dma_channel_wait_for_finish_blocking(dma_tx_channel);

    //channel_config_set_write_increment(&dma_rx_config, true);

    while (spi_is_readable(spi0))
        ret = spi_get_hw(spi0)->dr;

    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS)
        tight_loop_contents();

    while (spi_is_readable(spi0))
        ret = spi_get_hw(spi0)->dr;

    //spi_write_blocking(spi0, pBuf, len);

    return ret;
}

uint8_t w5500_read_byte()
{
    uint8_t data = 0;
    spi_read_blocking(spi0, 0xFF, &data, sizeof(data));
    return data;
}

void w5500_write_byte(uint8_t data)
{
    spi_write_blocking(spi0, &data, sizeof(data));
}

void w5500_lock(void)
{
    xSemaphoreTake(w5500_mutex, SYSTEM_WAIT_FOREVER);
}

void w5500_unlock(void)
{
    xSemaphoreGive(w5500_mutex);
}

/*_Noreturn static void w5500_spi_dma_handler(void)
{
    if (dma_channel_get_irq1_status(dma_rx_channel))
    {
        dma_channel_acknowledge_irq1(dma_rx_channel);
        xTaskNotify(socket_task_list[i].rx_task, W5500_TASK_INTERRUPT_OCCURRED, eSetBits);
    }
    else if (dma_channel_get_irq1_status(dma_tx_channel))
    {
        dma_channel_acknowledge_irq1(dma_tx_channel);
        xTaskNotify(socket_task_list[i].rx_task, W5500_TASK_INTERRUPT_OCCURRED, eSetBits);
    }
}*/

uint8_t w5500_find_unused_socket(void)
{
    for (uint8_t i = 0; i < W5500_SOCK_AMOUNT; i++)
    {
        if (w5500_socket_list[i] == W5500_SOCK_UNUSED)
        {
            return i;
        }
    }

    return W5500_SOCK_UNUSED;
}

void w5500_tcp_server_register_socket_tasks(uint8_t socket_number, TaskHandle_t rx_task, TaskHandle_t tx_task)
{
    if ((socket_number < W5500_SOCK_AMOUNT) &&
        (w5500_socket_list[socket_number] == W5500_SOCK_UNUSED) &&
        (w5500_socket_task_list[socket_number].rx_task == NULL) &&
        (w5500_socket_task_list[socket_number].tx_task == NULL))
    {
        w5500_socket_list[socket_number] = W5500_SOCK_USED;
        w5500_socket_task_list[socket_number].rx_task = rx_task;
        w5500_socket_task_list[socket_number].tx_task = tx_task;
    }
}

TaskHandle_t w5500_get_task_handle(void)
{
    return w5500_task;
}

_Noreturn void w5500_task_thread(void *pParams)
{
    uint32_t notificationValue = 0;

    while (1)
    {
        xTaskNotifyWait(0, W5500_TASK_INTERRUPT_OCCURRED, &notificationValue, pdMS_TO_TICKS(1000));

        if (notificationValue & W5500_TASK_INTERRUPT_OCCURRED)
        {
            w5500_lock();

            const uint8_t interrupt_status = w5500_get_global_sockets_irq_status();
            w5500_sock_irq_t irq_socket_status[W5500_SOCK_AMOUNT] = { 0 };

            for (uint32_t i = 0; i < W5500_SOCK_AMOUNT; i++)
            {
                if ((interrupt_status & (1 << i)) &&
                    (w5500_socket_task_list[i].rx_task != NULL) &&
                    (w5500_socket_task_list[i].tx_task != NULL))
                {
                    irq_socket_status[i] = w5500_get_socket_irq_status(i);
                    w5500_clear_socket_all_irq_status(i);
                }
            }

            w5500_unlock();

            uint32_t ack_mask = 0;

            for (uint32_t i = 0; i < W5500_SOCK_AMOUNT; i++)
            {
                if ((interrupt_status & (1 << i)) &&
                    (w5500_socket_task_list[i].rx_task != NULL) &&
                    (w5500_socket_task_list[i].tx_task != NULL))
                {
                    const uint8_t tx_task_notify_mask = W5500_SOCK_IRQ_SENDOK | W5500_SOCK_IRQ_TIMEOUT;

                    if (irq_socket_status[i] & tx_task_notify_mask)
                    {
                        //ack_mask |= W5500_TASK_TX_ACK(i);

                        xTaskNotify(w5500_socket_task_list[i].tx_task, irq_socket_status[i] & tx_task_notify_mask, eSetBits);
                    }

                    if (irq_socket_status[i] != tx_task_notify_mask)
                    {
                        ack_mask |= W5500_TASK_RX_ACK(i);
                        xTaskNotify(w5500_socket_task_list[i].rx_task, W5500_TASK_INTERRUPT_OCCURRED, eSetBits);
                    }
                }
            }
            do
            {
                xTaskNotifyWait(0, ack_mask, &notificationValue, SYSTEM_WAIT_FOREVER);

                ack_mask &= ~notificationValue;
            }
            while (ack_mask != 0);

            irq_set_enabled(IO_IRQ_BANK0, true);
        }
    }
}

void w5500_task_init(void)
{
    uint8_t memsize[2][8] = { { 8, 8, 0, 0, 0, 0, 0, 0 }, { 8, 8, 0, 0, 0, 0, 0, 0 } };

    uint8_t ip[4] = { 0 };
    uint8_t nm[4] = { 0 };
    uint8_t gw[4] = { 0 };
    uint8_t mac[6] = { 0 };

    wiz_NetInfo wiznet_info = { .dns = {0, 0, 0, 0},
                                .dhcp = NETINFO_STATIC };

    device_settings_get_value(DEVICE_SETTINGS_IP,   (uint8_t *) wiznet_info.ip, sizeof(wiznet_info.ip));
    device_settings_get_value(DEVICE_SETTINGS_NETMASK,(uint8_t *) wiznet_info.sn, sizeof(wiznet_info.sn));
    device_settings_get_value(DEVICE_SETTINGS_GATEWAY,(uint8_t *) wiznet_info.gw, sizeof(wiznet_info.gw));
    device_settings_get_value(DEVICE_SETTINGS_MAC,(uint8_t *) wiznet_info.mac, sizeof(wiznet_info.mac));

    memset(w5500_socket_list, W5500_SOCK_UNUSED, sizeof(w5500_socket_list));

    dma_rx_channel = dma_claim_unused_channel(true);
    dma_rx_config = dma_channel_get_default_config(dma_rx_channel);
    channel_config_set_transfer_data_size(&dma_rx_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_rx_config, spi_get_dreq(spi0, false));
    channel_config_set_read_increment(&dma_rx_config, false);
    channel_config_set_write_increment(&dma_rx_config, true);
    channel_config_set_enable(&dma_rx_config, true);

    dma_tx_channel = dma_claim_unused_channel(true);
    dma_tx_config = dma_channel_get_default_config(dma_tx_channel);
    channel_config_set_transfer_data_size(&dma_tx_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_tx_config, spi_get_dreq(spi0, true));
    channel_config_set_read_increment(&dma_tx_config, true);
    channel_config_set_write_increment(&dma_tx_config, false);
    channel_config_set_enable(&dma_tx_config, true);

    w5500_mutex = xSemaphoreCreateMutex();

    spi_init(spi0, 60 * 1000 * 1000);
    gpio_set_function(PIN_ETH_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_ETH_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_ETH_SCLK, GPIO_FUNC_SPI);

    gpio_init(PIN_ETH_CS);
    gpio_set_dir(PIN_ETH_CS, GPIO_OUT);
    gpio_put(PIN_ETH_CS, 1);

    gpio_init(PIN_ETH_RST);
    gpio_set_dir(PIN_ETH_RST, GPIO_OUT);
    gpio_put(PIN_ETH_RST, 1);

    gpio_init(PIN_ETH_INT);
    gpio_set_dir(PIN_ETH_INT, GPIO_IN);
    gpio_pull_up(PIN_ETH_INT);

    gpio_set_irq_enabled_with_callback(PIN_ETH_INT, GPIO_IRQ_LEVEL_LOW, true, w5500_int_callback);

    reg_wizchip_cs_cbfunc(w5500_select, w5500_deselect);
    reg_wizchip_spi_cbfunc(w5500_read_byte, w5500_write_byte);
    reg_wizchip_spiburst_cbfunc(w5500_read_burst, w5500_write_burst);
    //reg_wizchip_cris_cbfunc(w5500_lock, w5500_unlock);

    if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1)
    {
        while (1)
        {
        }
    }

    wiz_NetInfo getInfoTest = { 0 };

    ctlnetwork(CN_SET_NETINFO, (void*) &wiznet_info);

    ctlnetwork(CN_GET_NETINFO, (void*) &getInfoTest);

    BaseType_t status = xTaskCreate(w5500_task_thread,
                                    "w5500_task",
                                    configMINIMAL_STACK_SIZE,
                                    NULL,
                                    SYSTEM_PRIORITY_HIGHEST,
                                    &w5500_task);

    vTaskCoreAffinitySet(w5500_task, 0x01);
}

