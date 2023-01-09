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
#include "tcp_server.h"
#include "target_uart_task.h"

#define UART_RX_BUF_SIZE            (1024)
#define UART_RX_BUF_SIZE_IN_BYTES   (UART_RX_BUF_SIZE*sizeof(char))
#define UART_TX_BUF_SIZE            (UART_RX_BUF_SIZE)
#define UART_TX_BUF_SIZE_IN_BYTES   (UART_TX_BUF_SIZE*sizeof(char))

#define UART_DMA_RX_CHANNEL_COUNT   (2)

typedef struct
{
    int dma_rx_channel[UART_DMA_RX_CHANNEL_COUNT];
    int dma_rx_paired_channel[UART_DMA_RX_CHANNEL_COUNT];
    int dma_tx_channel;
    dma_channel_config dma_rx_config[UART_DMA_RX_CHANNEL_COUNT];
    dma_channel_config dma_tx_config;
    uint32_t dma_irq;
    __attribute__((aligned(UART_RX_BUF_SIZE_IN_BYTES)))
    char rx_buffer[UART_DMA_RX_CHANNEL_COUNT][UART_RX_BUF_SIZE];
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
} uart_thread_info_t;

typedef void (*uart_rx_handler_t)(void);
typedef bool (*uart_timeout_handler_t)(void);

void uart_channel_0_rx_interrupt(void);
bool uart_channel_0_timeout_interrupt(void);

void uart_channel_1_rx_interrupt(void);
bool uart_channel_1_timeout_interrupt(void);

static uart_inst_t *uart_hw_list[PLATFORM_USB_UART_INTERFACE_NUM] = {uart1, uart0};
static const uint32_t uart_dma_irq_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DMA_IRQ_0, DMA_IRQ_0};
static const uint32_t uart_uart_irq_list[PLATFORM_USB_UART_INTERFACE_NUM] = {UART1_IRQ, UART0_IRQ};

static const uart_rx_handler_t uart_rx_handler_list[PLATFORM_USB_UART_INTERFACE_NUM] = {uart_channel_0_rx_interrupt, uart_channel_1_rx_interrupt};
static const uart_timeout_handler_t uart_timeout_handler_list[PLATFORM_USB_UART_INTERFACE_NUM] = {uart_channel_0_timeout_interrupt, uart_channel_1_timeout_interrupt};

static const uint32_t uart_dreq_rx_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DREQ_UART1_RX, DREQ_UART0_RX};
static const uint32_t uart_dreq_tx_list[PLATFORM_USB_UART_INTERFACE_NUM] = {DREQ_UART1_TX, DREQ_UART0_TX};
static uart_thread_info_t uart_thread_info_list[PLATFORM_USB_UART_INTERFACE_NUM] = {0 };
static uint32_t uart_led_list[PLATFORM_USB_UART_INTERFACE_NUM] = {PIN_TARGET_1_LED_SER, PIN_TARGET_2_LED_SER };
static uint32_t uart_pins_list[PLATFORM_USB_UART_INTERFACE_NUM][2] = {{PIN_TARGET1_TX, PIN_TARGET1_RX}, {PIN_TARGET2_TX, PIN_TARGET2_RX}};

void uart_rx_callback(const uint8_t channel);
void uart_dma_handler(void);
bool uart_timeout_callback(const uint8_t channel);

void uart_tx_thread(void *pParams);
void uart_line_coding_thread(void *pParams);

void uart_rx_callback(const uint8_t channel)
{
    if (channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return;
    }

    system_pin_set(uart_thread_info_list[channel].serial_led);

    uart_set_irq_enables(uart_thread_info_list[channel].uart, false, false);

    for (uint8_t i = 0; i < UART_DMA_RX_CHANNEL_COUNT; i ++)
    {
        dma_channel_configure(uart_thread_info_list[channel].dma_rx_channel[i], &uart_thread_info_list[channel].dma_rx_config[i],
                              uart_thread_info_list[channel].rx_buffer[i],   // dst
                              &(uart_get_hw(uart_thread_info_list[channel].uart)->dr),  // src
                              UART_RX_BUF_SIZE,  // transfer count
                              (i == 0) ? true : false  // start immediately
        );

        dma_channel_acknowledge_irq0(uart_thread_info_list[channel].dma_rx_channel[i]);
        dma_channel_set_irq0_enabled(uart_thread_info_list[channel].dma_rx_channel[i], true);
    }

    hw_write_masked(&uart_get_hw(uart_thread_info_list[channel].uart)->dmacr, 1 << UART_UARTDMACR_RXDMAE_MSB,
                    UART_UARTDMACR_RXDMAE_BITS);

    memset(&(uart_thread_info_list[channel].rx_timeout_timer), 0, sizeof(uart_thread_info_list[channel].rx_timeout_timer));
    add_repeating_timer_ms(50, uart_thread_info_list[channel].rx_timeout_callback, NULL, &uart_thread_info_list[channel].rx_timeout_timer);
}

void uart_dma_handler(void)
{
    traceISR_ENTER();

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint32_t total_bytes = 0;

    for (uint32_t j = 0; j < PLATFORM_USB_UART_INTERFACE_NUM; j++)
    {
        uart_thread_info_t *config = &uart_thread_info_list[j];

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

            for (uint8_t i = 0; i < UART_DMA_RX_CHANNEL_COUNT; i ++)
            {
                dma_channel_configure(uart_thread_info_list[j].dma_rx_channel[i], &uart_thread_info_list[j].dma_rx_config[i],
                                      uart_thread_info_list[j].rx_buffer[i],   // dst
                                      &(uart_get_hw(uart_thread_info_list[j].uart)->dr),  // src
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
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    traceISR_EXIT();
}

bool uart_timeout_callback(const uint8_t channel)
{
    if (channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return false;
    }

    if (uart_thread_info_list[channel].dma_occured)
    {
        uart_thread_info_list[channel].dma_occured = false;
        return true;
    }
    else
    {
        uart_thread_info_t *config = &uart_thread_info_list[channel];

        hw_write_masked(&dma_channel_hw_addr(config->dma_rx_channel[0])->ctrl_trig,
                        0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                        DMA_CH0_CTRL_TRIG_EN_BITS);

        hw_write_masked(&dma_channel_hw_addr(config->dma_rx_channel[1])->ctrl_trig,
                        0 << DMA_CH1_CTRL_TRIG_EN_MSB,
                        DMA_CH1_CTRL_TRIG_EN_BITS);

        hw_write_masked(&uart_get_hw(config->uart)->dmacr, 0 << UART_UARTDMACR_RXDMAE_MSB,
                        UART_UARTDMACR_RXDMAE_BITS);

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        for (int i = 0; i < UART_DMA_RX_CHANNEL_COUNT; i++)
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
                    //config->rx_buffer[i][count_to_write - 1] |= 0x8000;
                }

                size_t written_bytes = xStreamBufferSendFromISR(uart_thread_info_list[channel].rx_stream,
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

            uart_set_irq_enables(uart_thread_info_list[channel].uart, true, false);
        }
        return false;
    }
}

void uart_channel_0_rx_interrupt(void)
{
    traceISR_ENTER();
    uart_rx_callback(0);
    traceISR_EXIT();
}

bool uart_channel_0_timeout_interrupt(void)
{
    traceISR_ENTER();
    const bool retVal = uart_timeout_callback(0);
    traceISR_EXIT();
    return retVal;
}

void uart_channel_1_rx_interrupt(void)
{
    traceISR_ENTER();
    uart_rx_callback(1);
    traceISR_EXIT();
}

bool uart_channel_1_timeout_interrupt(void)
{
    traceISR_ENTER();
    const bool retVal = uart_timeout_callback(1);
    traceISR_EXIT();
    return retVal;
}

void usb_uart_init(void)
{
    for (uint32_t i = 0; i < PLATFORM_USB_UART_INTERFACE_NUM; i++)
    {
        uart_thread_info_list[i].dma_rx_channel[0] = -1;
        uart_thread_info_list[i].dma_rx_channel[1] = -1;
        uart_thread_info_list[i].dma_tx_channel = -1;

        memset(uart_thread_info_list[i].rx_buffer[0], 0, sizeof(uart_thread_info_list[i].rx_buffer[0]));
        memset(uart_thread_info_list[i].rx_buffer[1], 0, sizeof(uart_thread_info_list[i].rx_buffer[1]));
        memset(uart_thread_info_list[i].tx_buffer, 0, sizeof(uart_thread_info_list[i].tx_buffer));

        uart_thread_info_list[i].rx_stream = NULL;
        uart_thread_info_list[i].tx_stream = NULL;
        uart_thread_info_list[i].line_coding_stream = NULL;

        gpio_set_function(uart_pins_list[i][0], GPIO_FUNC_UART);
        gpio_set_function(uart_pins_list[i][1], GPIO_FUNC_UART);
    }
}

void usb_uart_add_interface(target_uart_config_t *p_target_uart_config)
{
    const uint8_t uart_channel = p_target_uart_config->uart_channel;
    const target_uart_remote_side remote_side = p_target_uart_config->remote_side;

    if (uart_channel >= PLATFORM_USB_UART_INTERFACE_NUM)
    {
        return;
    }

    if ((uart_thread_info_list[uart_channel].dma_rx_channel[0] != -1) ||
        (uart_thread_info_list[uart_channel].dma_rx_channel[1] != -1) ||
        (uart_thread_info_list[uart_channel].dma_tx_channel != -1))
    {
        return;
    }

    uart_thread_info_list[uart_channel].serial_led = uart_led_list[uart_channel];

    uart_thread_info_list[uart_channel].tx_stream = xStreamBufferCreate(UART_TX_BUF_SIZE_IN_BYTES*2, UART_TX_BUF_SIZE_IN_BYTES);
    uart_thread_info_list[uart_channel].rx_stream = xStreamBufferCreate(UART_RX_BUF_SIZE_IN_BYTES*4, UART_RX_BUF_SIZE_IN_BYTES);
    uart_thread_info_list[uart_channel].line_coding_stream = xStreamBufferCreate(sizeof(cdc_line_coding_t)*2, sizeof(cdc_line_coding_t));

    if (remote_side == TARGET_UART_REMOTE_SIDE_USB)
    {
        usb_cdc_config_t *usb_uart_config = MemManager_Alloc(sizeof(usb_cdc_config_t));
        usb_uart_config->cdc_interface_number = p_target_uart_config->usb_config.cdc_interface_number;
        usb_uart_config->send_eot = false;
        usb_uart_config->rx_stream = uart_thread_info_list[uart_channel].tx_stream;
        usb_uart_config->tx_stream = uart_thread_info_list[uart_channel].rx_stream;
        usb_uart_config->line_coding_stream = uart_thread_info_list[uart_channel].line_coding_stream;

        usb_cdc_task_init(usb_uart_config);
    }
    else if (remote_side == TARGET_UART_REMOTE_SIDE_TELNET)
    {
        tcp_server_config_t *telnet_uart_config = MemManager_Alloc(sizeof(tcp_server_config_t));
        telnet_uart_config->socket_number = W5500_SOCK_UNUSED; // Claim first unused socket
        telnet_uart_config->port = p_target_uart_config->telnet_config.port;
        telnet_uart_config->rx_stream = uart_thread_info_list[uart_channel].tx_stream;
        telnet_uart_config->tx_stream = uart_thread_info_list[uart_channel].rx_stream;

        xStreamBufferSendFromISR(uart_thread_info_list[uart_channel].line_coding_stream, &(p_target_uart_config->telnet_config.line_coding),
                                 sizeof(p_target_uart_config->telnet_config.line_coding), SYSTEM_WAIT_DONT_BLOCK);

        tcp_server_add(telnet_uart_config);
    }
    else
    {
        //TODO: assert
    }

    uart_thread_info_list[uart_channel].dma_rx_channel[0] = dma_claim_unused_channel(true);
    uart_thread_info_list[uart_channel].dma_rx_channel[1] = dma_claim_unused_channel(true);
    uart_thread_info_list[uart_channel].dma_tx_channel    = dma_claim_unused_channel(true);

    uart_thread_info_list[uart_channel].dma_rx_paired_channel[0] = uart_thread_info_list[uart_channel].dma_rx_channel[1];
    uart_thread_info_list[uart_channel].dma_rx_paired_channel[1] = uart_thread_info_list[uart_channel].dma_rx_channel[0];

    uart_thread_info_list[uart_channel].dma_irq = uart_dma_irq_list[uart_channel];

    uart_thread_info_list[uart_channel].uart = uart_hw_list[uart_channel];
    uart_thread_info_list[uart_channel].uart_irq = uart_uart_irq_list[uart_channel];
    uart_thread_info_list[uart_channel].rx_callback = uart_rx_handler_list[uart_channel];
    uart_thread_info_list[uart_channel].rx_timeout_callback = uart_timeout_handler_list[uart_channel];

    dma_channel_config *config = &(uart_thread_info_list[uart_channel].dma_tx_config);
    *config = dma_channel_get_default_config(uart_thread_info_list[uart_channel].dma_tx_channel);

    channel_config_set_transfer_data_size(config, DMA_SIZE_8);
    channel_config_set_read_increment(config, true);
    channel_config_set_write_increment(config, false);
    channel_config_set_dreq(config, uart_dreq_tx_list[uart_channel]);

    irq_set_exclusive_handler(uart_thread_info_list[uart_channel].dma_irq, uart_dma_handler);
    irq_set_enabled(uart_thread_info_list[uart_channel].dma_irq, true);

    for (uint32_t i = 0; i < 2; i++)
    {
        config = &(uart_thread_info_list[uart_channel].dma_rx_config[i]);
        //int dma_channel = usb_uart_config_list[channel].dma_rx_channel[i];

        *config = dma_channel_get_default_config(uart_thread_info_list[uart_channel].dma_rx_channel[i]);

        channel_config_set_transfer_data_size(config, DMA_SIZE_8);
        channel_config_set_read_increment(config, false);
        channel_config_set_write_increment(config, true);

        if (i == 0)
        {
            channel_config_set_chain_to(config, uart_thread_info_list[uart_channel].dma_rx_paired_channel[i]);
        }
        channel_config_set_dreq(config, uart_dreq_rx_list[uart_channel]);
    }

    BaseType_t status;

    TaskHandle_t usb_uart_tx_thread_handle;
    status = xTaskCreate(uart_tx_thread,
                         "usb_uart_tx_thread",
                         configMINIMAL_STACK_SIZE,
                         &uart_thread_info_list[uart_channel],
                         SYSTEM_PRIORITY_HIGH,
                         &usb_uart_tx_thread_handle);

    TaskHandle_t usb_uart_line_coding_thread_handle;
    status = xTaskCreate(uart_line_coding_thread,
                         "usb_uart_line_coding_thread",
                         configMINIMAL_STACK_SIZE,
                         &uart_thread_info_list[uart_channel],
                         SYSTEM_PRIORITY_LOWEST,
                         &usb_uart_line_coding_thread_handle);

    uart_thread_info_list[uart_channel].tx_thread = usb_uart_tx_thread_handle;

    vTaskCoreAffinitySet(usb_uart_tx_thread_handle, 0x01);
    vTaskCoreAffinitySet(usb_uart_line_coding_thread_handle, 0x01);
}

_Noreturn void usb_uart_tx_thread(void *pParams)
{
    uart_thread_info_t *config = (uart_thread_info_t*)pParams;
    StreamBufferHandle_t usb_uart_thread_tx_stream = config->tx_stream;

    while(1)
    {
        const size_t received_bytes = xStreamBufferReceive(usb_uart_thread_tx_stream, config->tx_buffer, sizeof(config->tx_buffer),pdMS_TO_TICKS(50));
        if (received_bytes > 0)
        {
            system_pin_set(config->serial_led);
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
            system_pin_clear(config->serial_led);
        }
    }
}

_Noreturn void uart_line_coding_thread(void *pParams)
{
    uart_thread_info_t *config = (uart_thread_info_t*)pParams;
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

