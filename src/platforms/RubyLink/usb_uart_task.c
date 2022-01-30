#include "general.h"
#include "hardware/dma.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "pico/time.h"

#include "system_pins.h"

#include "FreeRTOS.h"
#include "tusb.h"
#include "stream_buffer.h"

#include "usb_cdc_task.h"
#include "usb_uart_task.h"

typedef uint16_t usb_uart_element_t;

#define UART_RX_BUF_SIZE            (128)
#define UART_RX_BUF_SIZE_IN_BYTES   (UART_RX_BUF_SIZE*sizeof(usb_uart_element_t))
#define UART_TX_BUF_SIZE            (UART_RX_BUF_SIZE)
#define UART_TX_BUF_SIZE_IN_BYTES   (UART_TX_BUF_SIZE*sizeof(char))

typedef struct
{
    int dma_rx_channel[2];
    int dma_rx_paired_channel[2];
    int dma_tx_channel;
    dma_channel_config dma_rx_config[2];
    dma_channel_config dma_tx_config;
    uint32_t dma_irq;
    __attribute__((aligned(UART_RX_BUF_SIZE_IN_BYTES)))
    usb_uart_element_t rx_buffer[2][UART_RX_BUF_SIZE];
    char tx_buffer[UART_TX_BUF_SIZE];

    uart_inst_t *uart;
    uint32_t uart_irq;
    void (*rx_callback)();

    repeating_timer_t rx_timeout_timer;
    bool (*rx_timeout_callback)();

    bool dma_occured;

    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t rx_stream;
    StreamBufferHandle_t line_coding_stream;

    TaskHandle_t tx_thread;

    uint32_t serial_led;
} usb_uart_config_t;

static uart_inst_t *usb_uart_interfaces_list[PLATFORM_USB_UART_INTERFACE_NUM] = {uart1, uart0};
static const uint32_t usb_uart_dma_irq_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DMA_IRQ_0, DMA_IRQ_0};
static const uint32_t usb_uart_uart_irq_list[PLATFORM_USB_UART_INTERFACE_NUM] = {UART1_IRQ, UART0_IRQ};
static const uint32_t usb_uart_uart_dreq_rx_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DREQ_UART1_RX, DREQ_UART0_RX};
static const uint32_t usb_uart_uart_dreq_tx_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DREQ_UART1_TX, DREQ_UART0_TX};
static usb_uart_config_t usb_uart_config_list[PLATFORM_USB_UART_INTERFACE_NUM] = { 0 };
static uint32_t usb_uart_led_list[PLATFORM_USB_UART_INTERFACE_NUM] = { PIN_TARGET_1_LED_SER, PIN_TARGET_2_LED_SER };

void usb_uart_rx_callback(const uint8_t channel);
void usb_uart_dma_handler(void);
bool usb_uart_timeout_callback(const uint8_t channel);

void usb_uart_1_rx_interrupt(void);
bool usb_uart_1_timeout_interrupt(void);

void usb_uart_tx_thread(void *pParams);
void usb_uart_line_coding_thread(void *pParams);

void usb_uart_rx_callback(const uint8_t channel)
{
    if (channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    system_pin_set(usb_uart_config_list[channel].serial_led);

    uart_set_irq_enables(usb_uart_config_list[channel].uart, false, false);

    for (uint8_t i = 0; i < 2; i ++)
    {
        dma_channel_configure(usb_uart_config_list[channel].dma_rx_channel[i], &usb_uart_config_list[channel].dma_rx_config[i],
                              usb_uart_config_list[channel].rx_buffer[i],   // dst
                              &(uart_get_hw(usb_uart_config_list[channel].uart)->dr),  // src
                              UART_RX_BUF_SIZE,  // transfer count
                              (i == 0) ? true : false  // start immediately
        );

        dma_channel_acknowledge_irq0(usb_uart_config_list[channel].dma_rx_channel[i]);
        dma_channel_set_irq0_enabled(usb_uart_config_list[channel].dma_rx_channel[i], true);
    }

    hw_write_masked(&uart_get_hw(usb_uart_config_list[channel].uart)->dmacr, 1 << UART_UARTDMACR_RXDMAE_MSB,
                    UART_UARTDMACR_RXDMAE_BITS);

    memset(&(usb_uart_config_list[channel].rx_timeout_timer), 0, sizeof(usb_uart_config_list[channel].rx_timeout_timer));
    add_repeating_timer_ms(50, usb_uart_config_list[channel].rx_timeout_callback, NULL, &usb_uart_config_list[channel].rx_timeout_timer);
}

void usb_uart_dma_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint32_t total_bytes = 0;

    for (uint32_t j = 0; j < PLATFORM_USB_UART_INTERFACE_NUM; j++)
    {
        usb_uart_config_t *config = &(usb_uart_config_list[j]);

        if ((config->dma_rx_channel[0] == -1) || (config->dma_rx_channel[1] == -1))
        {
            continue;
        }

        if (dma_channel_get_irq0_status(config->dma_rx_channel[0]))
        {
            const int dma_channel = config->dma_rx_channel[0];
            config->dma_occured = true;

            dma_channel_acknowledge_irq0(dma_channel);

            size_t written_bytes = xStreamBufferSendFromISR(config->rx_stream, config->rx_buffer[0],
                                                            UART_RX_BUF_SIZE_IN_BYTES, &xHigherPriorityTaskWoken);
            if (written_bytes != UART_RX_BUF_SIZE_IN_BYTES)
            {
                total_bytes = 0;
            }
            else
            {
                total_bytes += written_bytes;
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else if (dma_channel_get_irq0_status(config->dma_rx_paired_channel[0]))
        {
            const int dma_channel = config->dma_rx_paired_channel[0];
            config->dma_occured = true;

            dma_channel_acknowledge_irq0(dma_channel);

            for (uint8_t i = 0; i < 2; i ++)
            {
                dma_channel_configure(usb_uart_config_list[j].dma_rx_channel[i], &usb_uart_config_list[j].dma_rx_config[i],
                                      usb_uart_config_list[j].rx_buffer[i],   // dst
                                      &(uart_get_hw(usb_uart_config_list[j].uart)->dr),  // src
                                      UART_RX_BUF_SIZE,  // transfer count
                                      (i == 0) ? true : false            // start immediately
                );
            }
            size_t written_bytes = xStreamBufferSendFromISR(config->rx_stream, config->rx_buffer[1],
                                                            UART_RX_BUF_SIZE_IN_BYTES, &xHigherPriorityTaskWoken);
            if (written_bytes != UART_RX_BUF_SIZE_IN_BYTES)
            {
                total_bytes = 0;
            }
            else
            {
                total_bytes += written_bytes;
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else if (dma_channel_get_irq0_status(config->dma_tx_channel))
        {
            dma_channel_acknowledge_irq0(config->dma_tx_channel);
            dma_channel_set_irq0_enabled(config->dma_tx_channel, false);

            hw_write_masked(&uart_get_hw(config->uart)->dmacr, 0,
                            UART_UARTDMACR_TXDMAE_BITS);
            xTaskNotifyFromISR(config->tx_thread, USB_UART_TASK_TX_COMPLETE, eSetBits, &xHigherPriorityTaskWoken);
        }
    }
}

bool usb_uart_timeout_callback(const uint8_t channel)
{
    if (channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return false;
    }

    if (usb_uart_config_list[channel].dma_occured)
    {
        usb_uart_config_list[channel].dma_occured = false;
        return true;
    }
    else
    {
        usb_uart_config_t *config = &usb_uart_config_list[channel];

        hw_write_masked(&dma_channel_hw_addr(config->dma_rx_channel[0])->ctrl_trig,
                        0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                        DMA_CH0_CTRL_TRIG_EN_BITS);

        hw_write_masked(&dma_channel_hw_addr(config->dma_rx_channel[1])->ctrl_trig,
                        0 << DMA_CH1_CTRL_TRIG_EN_MSB,
                        DMA_CH1_CTRL_TRIG_EN_BITS);

        hw_write_masked(&uart_get_hw(config->uart)->dmacr, 0 << UART_UARTDMACR_RXDMAE_MSB,
                        UART_UARTDMACR_RXDMAE_BITS);

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        for (int i = 0; i < 2; i++)
        {
            int dma_channel = config->dma_rx_channel[i];
            int dma_paired_channel = config->dma_rx_paired_channel[i];
            const uint32_t transfer_count = dma_channel_hw_addr(dma_channel)->transfer_count;
            const uint32_t count_to_write = UART_RX_BUF_SIZE - transfer_count;
            const uint32_t byte_to_write = count_to_write * sizeof(config->rx_buffer[i][0]);

            uint32_t byte_count = 0;

            if ((count_to_write > 0) && (count_to_write < UART_RX_BUF_SIZE))
            {
                if (dma_channel_hw_addr(dma_paired_channel)->transfer_count == 0)
                {
                    config->rx_buffer[i][count_to_write - 1] |= 0x8000;
                }

                size_t written_bytes = xStreamBufferSendFromISR(usb_uart_config_list[channel].rx_stream,
                                                                config->rx_buffer[i], byte_to_write,
                                                                &xHigherPriorityTaskWoken);
                if (written_bytes != byte_to_write)
                {
                    byte_count = 0;
                }
            }

            dma_channel_acknowledge_irq0(dma_channel);
            dma_channel_set_irq0_enabled(dma_channel, false);
            dma_channel_abort(dma_channel);

            uart_set_irq_enables(uart1, true, false);
        }
        return false;
    }
}

void usb_uart_1_rx_interrupt(void)
{
    usb_uart_rx_callback(0);
}

bool usb_uart_1_timeout_interrupt(void)
{
    return usb_uart_timeout_callback(0);
}

void usb_uart_init(void)
{
    for (uint32_t i = 0; i < PLATFORM_USB_UART_INTERFACE_NUM; i++)
    {
        usb_uart_config_list[i].dma_rx_channel[0] = -1;
        usb_uart_config_list[i].dma_rx_channel[1] = -1;
        usb_uart_config_list[i].dma_tx_channel = -1;

        memset(usb_uart_config_list[i].rx_buffer[0], 0, sizeof(usb_uart_config_list[i].rx_buffer[0]));
        memset(usb_uart_config_list[i].rx_buffer[1], 0, sizeof(usb_uart_config_list[i].rx_buffer[1]));
        memset(usb_uart_config_list[i].tx_buffer, 0, sizeof(usb_uart_config_list[i].tx_buffer));

        usb_uart_config_list[i].rx_stream = NULL;
        usb_uart_config_list[i].tx_stream = NULL;
        usb_uart_config_list[i].line_coding_stream = NULL;
    }
}

void usb_uart_add_interface(uint8_t channel)
{
    if (channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return;
    }

    if ((usb_uart_config_list[channel].dma_rx_channel[0] != -1) ||
        (usb_uart_config_list[channel].dma_rx_channel[1] != -1) ||
        (usb_uart_config_list[channel].dma_tx_channel != -1))
    {
        return;
    }

    usb_uart_config_list[channel].serial_led = usb_uart_led_list[channel];

    usb_uart_config_list[channel].tx_stream = xStreamBufferCreate(UART_TX_BUF_SIZE_IN_BYTES*4, UART_TX_BUF_SIZE_IN_BYTES);
    usb_uart_config_list[channel].rx_stream = xStreamBufferCreate(UART_RX_BUF_SIZE_IN_BYTES*4, UART_RX_BUF_SIZE_IN_BYTES);
    usb_uart_config_list[channel].line_coding_stream = xStreamBufferCreate(sizeof(cdc_line_coding_t)*2, sizeof(cdc_line_coding_t));

    usb_cdc_test_task_init(1, "usb_uart_rx_thread", "usb_uart_tx_thread", usb_uart_config_list[channel].tx_stream, usb_uart_config_list[channel].rx_stream, usb_uart_config_list[channel].line_coding_stream);

    usb_uart_config_list[channel].dma_rx_channel[0] = dma_claim_unused_channel(true);
    usb_uart_config_list[channel].dma_rx_channel[1] = dma_claim_unused_channel(true);
    usb_uart_config_list[channel].dma_tx_channel    = dma_claim_unused_channel(true);

    usb_uart_config_list[channel].dma_rx_paired_channel[0] = usb_uart_config_list[channel].dma_rx_channel[1];
    usb_uart_config_list[channel].dma_rx_paired_channel[1] = usb_uart_config_list[channel].dma_rx_channel[0];

    usb_uart_config_list[channel].dma_irq = usb_uart_dma_irq_list[channel];

    usb_uart_config_list[channel].uart = usb_uart_interfaces_list[channel];
    usb_uart_config_list[channel].uart_irq = usb_uart_uart_irq_list[channel];
    usb_uart_config_list[channel].rx_callback = usb_uart_1_rx_interrupt;

    usb_uart_config_list[channel].rx_timeout_callback = usb_uart_1_timeout_interrupt;

    dma_channel_config *config = &(usb_uart_config_list[channel].dma_tx_config);
    *config = dma_channel_get_default_config(usb_uart_config_list[channel].dma_tx_channel);

    channel_config_set_transfer_data_size(config, DMA_SIZE_8);
    channel_config_set_read_increment(config, true);
    channel_config_set_write_increment(config, false);
    channel_config_set_dreq(config, usb_uart_uart_dreq_tx_list[channel]);

    irq_set_exclusive_handler(usb_uart_config_list[channel].dma_irq, usb_uart_dma_handler);
    irq_set_enabled(usb_uart_config_list[channel].dma_irq, true);

    for (uint32_t i = 0; i < 2; i++)
    {
        config = &(usb_uart_config_list[channel].dma_rx_config[i]);
        //int dma_channel = usb_uart_config_list[channel].dma_rx_channel[i];

        *config = dma_channel_get_default_config(usb_uart_config_list[channel].dma_rx_channel[i]);

        channel_config_set_transfer_data_size(config, DMA_SIZE_16);
        channel_config_set_read_increment(config, false);
        channel_config_set_write_increment(config, true);

        if (i == 0)
        {
            channel_config_set_chain_to(config, usb_uart_config_list[channel].dma_rx_paired_channel[i]);
        }
        channel_config_set_dreq(config, usb_uart_uart_dreq_rx_list[channel]);
    }

    BaseType_t status;

    TaskHandle_t usb_uart_tx_thread_handle;
    status = xTaskCreate(usb_uart_tx_thread,
                         "usb_uart_rx_thread",
                         configMINIMAL_STACK_SIZE,
                         &usb_uart_config_list[channel],
                         SYSTEM_PRIORITY_LOW,
                         &usb_uart_tx_thread_handle);

    TaskHandle_t usb_uart_line_coding_thread_handle;
    status = xTaskCreate(usb_uart_line_coding_thread,
                         "usb_uart_line_coding_thread",
                         configMINIMAL_STACK_SIZE,
                         &usb_uart_config_list[channel],
                         SYSTEM_PRIORITY_LOW,
                         &usb_uart_line_coding_thread_handle);

    usb_uart_config_list[channel].tx_thread = usb_uart_tx_thread_handle;

    vTaskCoreAffinitySet(usb_uart_tx_thread_handle, 0x01);
    vTaskCoreAffinitySet(usb_uart_line_coding_thread_handle, 0x01);
}

_Noreturn void usb_uart_tx_thread(void *pParams)
{
    usb_uart_config_t *config = (usb_uart_config_t*)pParams;
    StreamBufferHandle_t usb_uart_thread_tx_stream = config->tx_stream;

    while(1)
    {
        const size_t received_bytes = xStreamBufferReceive(usb_uart_thread_tx_stream, config->tx_buffer, sizeof(config->tx_buffer),pdMS_TO_TICKS(50));
        if (received_bytes > 0)
        {
            system_pin_set(PIN_TARGET_1_LED_SER);
            if (uart_is_enabled(config->uart))
            {
                dma_channel_configure(config->dma_tx_channel, &config->dma_tx_config,
                                      &(uart_get_hw(config->uart)->dr),
                                      config->tx_buffer,
                                      received_bytes / sizeof(config->tx_buffer[0]),  // transfer count
                                      true            // start immediately
                );

                dma_channel_acknowledge_irq0(config->dma_tx_channel);
                dma_channel_set_irq0_enabled(config->dma_tx_channel, true);

                hw_write_masked(&uart_get_hw(config->uart)->dmacr, 1 << UART_UARTDMACR_TXDMAE_MSB,
                                UART_UARTDMACR_TXDMAE_BITS);

                xTaskNotifyWait(0, USB_UART_TASK_TX_COMPLETE, NULL, SYSTEM_WAIT_FOREVER);
            }

            if (xStreamBufferBytesAvailable(usb_uart_thread_tx_stream) == 0)
            {
            }
        }
        else
        {
            system_pin_clear(PIN_TARGET_1_LED_SER);
        }
    }
}

_Noreturn void usb_uart_line_coding_thread(void *pParams)
{
    usb_uart_config_t *config = (usb_uart_config_t*)pParams;
    StreamBufferHandle_t line_coding_stream = config->line_coding_stream;

    cdc_line_coding_t line_coding;

    while(1)
    {
        const size_t received_bytes = xStreamBufferReceive(line_coding_stream, &line_coding, sizeof(line_coding), SYSTEM_WAIT_FOREVER);
        if (received_bytes == sizeof(line_coding))
        {
            for (uint8_t i = 0; i < 2; i ++)
            {
                dma_channel_set_irq0_enabled(config->dma_rx_channel[i], false);
                dma_channel_abort(config->dma_rx_channel[i]);

                hw_write_masked(&dma_channel_hw_addr(config->dma_rx_channel[i])->ctrl_trig, 0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                                DMA_CH0_CTRL_TRIG_EN_BITS);
            }

            dma_channel_set_irq0_enabled(config->dma_tx_channel, false);
            dma_channel_abort(config->dma_tx_channel);
            hw_write_masked(&dma_channel_hw_addr(config->dma_tx_channel)->ctrl_trig, 0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                            DMA_CH0_CTRL_TRIG_EN_BITS);

            uart_deinit(config->uart);

            uart_init(config->uart, line_coding.bit_rate);
            uart_set_hw_flow(config->uart, false, false);
            uart_set_format(config->uart, line_coding.data_bits, line_coding.stop_bits + 1, line_coding.parity);

            irq_set_exclusive_handler(config->uart_irq, config->rx_callback);
            irq_set_enabled(config->uart_irq, true);

            hw_write_masked(&uart_get_hw(config->uart)->dmacr, 0,
                            UART_UARTDMACR_RXDMAE_BITS | UART_UARTDMACR_TXDMAE_BITS);

            hw_write_masked(&uart_get_hw(config->uart)->ifls, 2 << UART_UARTIFLS_RXIFLSEL_LSB,
                            UART_UARTIFLS_RXIFLSEL_BITS);

            hw_write_masked(&uart_get_hw(config->uart)->ifls, 2 << UART_UARTIFLS_TXIFLSEL_LSB,
                            UART_UARTIFLS_TXIFLSEL_BITS);

            uart_set_irq_enables(config->uart, true, false);
        }
    }
}

