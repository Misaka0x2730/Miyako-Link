/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2011  Black Sphere Technologies Ltd.
 * Written by Gareth McMullin <gareth@blacksphere.co.nz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Provides main entry point.  Initialise subsystems and enter GDB
 * protocol loop.
 */

#include "general.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#include "morse.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "system_func.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "atomic.h"
#include "mem_manager.h"
#include "device_settings.h"
#include "wizchip_conf.h"
#include "usb_task.h"
#include "usb_cdc_task.h"
#include "logging.h"
#include "w5500_task.h"
#include "tcp_server.h"
#include "led.h"

#include "system_pins.h"
#include "stream_buffer.h"
#include "target_uart_task.h"

#define GDB_THREAD_NAME_LENGTH	(20)

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );

void *gdb_thread(void *arg)
{
	/*int iface_number = (int*)arg;

	add_pid_to_list();
	cdc_thread_init();
	set_interface_number(iface_number);

	while (true) {
		volatile struct exception e;
		TRY_CATCH(e, EXCEPTION_ALL) {
			gdb_main();
		}
		if (e.type) {
			gdb_putpacketz("EFF");
			target_list_free();
			morse(get_interface_number(), "TARGET LOST.", 1);
		}
	}*/
}

void *morse_thread(void *arg)
{
	int number = *(int*)arg;

	while(1) {
		//SET_ERROR_STATE(morse_update(number));
		//xtimer_usleep(MORSE_PERIOD*1000);
	}
}

void gdb_threads_start(void)
{
	/*char thread_name[GDB_THREAD_NAME_LENGTH] = {0};
	static int interface_list[INTERFACE_NUMBER] = {0};

	for(int i = 0; i < INTERFACE_NUMBER; ++i) {
		snprintf(thread_name, GDB_THREAD_NAME_LENGTH, "morse_thread_%d", i+1);
		interface_list[i] = i;

		thread_create(morse_stack[i], sizeof(morse_stack[i]),
						MORSE_THREAD_PRIO, 0,
						morse_thread, &interface_list[i], thread_name);
	}

	for(int i = 0; i < MAX_GDB_NUMBER; ++i) {
		snprintf(thread_name, GDB_THREAD_NAME_LENGTH, "gdb_thread_%d", i+1);
		thread_create(gdb_stack[i], sizeof(gdb_stack[i]),
						GDB_THREAD_PRIO-i, 0,
						gdb_thread, &interface_list[i], thread_name);
	}*/
}

#define MUX_ALARM_NUMBER        (0)
#define MUX_TIMER_IRQ           (TIMER_IRQ_0)
#define MUX_SHIFT_GPIO_NUMBER   (13)
#define MUX_TIMER_MAX_TIMEOUT   (5) //in ticks
#define MUX_TIMER_ALARM_TIMEOUT (5)

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */

    /* Force an assert. */
    configASSERT( ( volatile void * ) NULL );
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
      state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
      Note that, as the array is necessarily of type StackType_t,
      configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
      task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
      Note that, as the array is necessarily of type StackType_t,
      configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#include "bsp/board.h"

_Noreturn void TestLedTaskThread (void *pParams)
{
    while(1)
    {
        //LogWarn("Led Task");
        //mux_gpio_mask_set(0xFFFF);
        //vTaskDelay(500);
        vTaskDelay(500);
        /*system_pin_set_value(PIN_TARGET_1_LED_ACT, true);
        system_pin_set_value(PIN_TARGET_1_LED_ERR, true);
        system_pin_set_value(PIN_TARGET_1_LED_SER, true);
        vTaskDelay(500);
        system_pin_set_value(PIN_TARGET_1_LED_ACT, false);
        system_pin_set_value(PIN_TARGET_1_LED_ERR, false);
        system_pin_set_value(PIN_TARGET_1_LED_SER, false);*/
        //LogWarn("Led Task");
    }
}

QueueHandle_t cli_rx_queue;
QueueHandle_t cli_tx_queue;

_Noreturn void LogTaskThread(void *pParams)
{
    char *message = NULL;

    while(1)
    {
        message = System_ReceiveLog();
        if (message != NULL)
        {
            const uint32_t len = strlen(message);
            for (uint32_t i = 0; i < len; i++)
            {
                const char character = *(message + i);
                const uint16_t data = (i == (len - 1)) ? (0x8000 | (uint8_t)character) : (uint8_t)character;
                //LogWarn("GDB putchar");
                xQueueSend(cli_tx_queue, (void*)&data, SYSTEM_WAIT_DONT_BLOCK);
            }
            MemManager_Free(message);
        }
    }
}

void wizchip_select(void)
{
    gpio_put(1, 0);
}

void wizchip_deselect(void)
{
    gpio_put(1, 1);
}

void wizchip_read(uint8_t* pBuf, uint16_t len)
{
    spi_read_blocking(spi0, 0xFF, pBuf, len);
}

void wizchip_write(uint8_t* pBuf, uint16_t len)
{
    spi_write_blocking(spi0, pBuf, len);
}

uint8_t wizchip_read_byte()
{
    uint8_t data = 0;
    spi_read_blocking(spi0, 0xFF, &data, sizeof(data));
    return data;
}

void wizchip_write_byte(uint8_t data)
{
    spi_write_blocking(spi0, &data, sizeof(data));
}


static SemaphoreHandle_t ethMutex;

void wizchip_lock(void)
{
    xSemaphoreTake(ethMutex, SYSTEM_WAIT_FOREVER);
}

void wizchip_unlock(void)
{
    xSemaphoreGive(ethMutex);
}

StreamBufferHandle_t rx_stream;
StreamBufferHandle_t tx_stream;

void EthTaskThread(void *pParams)
{
    /*uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };

    wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
            .ip = {192, 168, 1, 2},
            .sn = {255, 255, 255, 0},
            .gw = {192, 168, 1, 1},
            .dns = {0, 0, 0, 0},
            .dhcp = NETINFO_STATIC };

    ethMutex = xSemaphoreCreateMutex();

    spi_init(spi0, 33 * 1000 * 1000);
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

    gpio_set_irq_enabled_with_callback(PIN_ETH_INT, GPIO_IRQ_EDGE_FALL, true, w5500_int_callback);

    uint8_t mac[6] = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef};

    memcpy(gWIZNETINFO.mac, mac, sizeof(mac));

    vTaskDelay(1000);

    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    reg_wizchip_spi_cbfunc(wizchip_read_byte, wizchip_write_byte);
    reg_wizchip_spiburst_cbfunc(wizchip_read, wizchip_write);
    reg_wizchip_cris_cbfunc(wizchip_lock, wizchip_unlock);

    if (ctlwizchip(CW_INIT_WIZCHIP, (void*) memsize) == -1)
    {
        while (1)
        {
            vTaskDelay(1000);
        }
    }

    wiz_NetInfo getInfoTest = { 0 };

    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);

    vTaskDelay(1000);

    ctlnetwork(CN_GET_NETINFO, (void*) &getInfoTest);

    //tcp_server_task_init(0, "usb_gdb_rx_thread", "usb_gdb_tx_thread", rx_stream, tx_stream);

    tcp_server_add_interface(0, 10032, "tcp_server_rx_thread", "tcp_server_tx_thread", rx_stream, tx_stream);

    while (1)
    {
        vTaskDelay(1000);
    }*/
}

#include "gdb_hostio.h"

struct target_controller gdb_controller[2];
QueueHandle_t rx_queue;
QueueHandle_t tx_queue;


StreamBufferHandle_t line_coding_stream;

StreamBufferHandle_t usb_uart_rx_stream;
StreamBufferHandle_t usb_uart_tx_stream;
StreamBufferHandle_t usb_uart_line_coding_stream;

TaskHandle_t usb_cdc_task_handle = NULL;

unsigned char gdb_tx_buf[256] = { 0 };
uint32_t gdb_tx_count = 0;

void gdb_if_putchar(unsigned char c, int flush)
{
    gdb_tx_buf[gdb_tx_count++] = c;
    if ((gdb_tx_count == sizeof(gdb_tx_buf)) || (flush))
    {
        //const uint16_t data = flush ? (0x8000 | (uint8_t)c) : (uint8_t)c;
        xStreamBufferSend(tx_stream, gdb_tx_buf, gdb_tx_count, SYSTEM_WAIT_FOREVER);
        gdb_tx_count = 0;
    }
}

unsigned char gdb_rx_buf[256] = { 0 };
uint32_t gdb_rx_count = 0;
uint32_t gdb_rx_last_pos = 0;

unsigned char gdb_if_getchar(void)
{
    if (gdb_rx_count)
    {
        gdb_rx_count--;
        return gdb_rx_buf[gdb_rx_last_pos++];
    }

    size_t received_bytes = xStreamBufferReceive(rx_stream, gdb_rx_buf, sizeof(gdb_rx_buf), SYSTEM_WAIT_FOREVER);

    gdb_rx_count = received_bytes - 1;
    gdb_rx_last_pos = 1;
    return gdb_rx_buf[0];
}

unsigned char gdb_if_getchar_to(int timeout)
{
    if (gdb_rx_count)
    {
        gdb_rx_count--;
        return gdb_rx_buf[gdb_rx_last_pos++];
    }

    size_t received_bytes = xStreamBufferReceive(rx_stream, gdb_rx_buf, sizeof(gdb_rx_buf), pdMS_TO_TICKS(timeout));

    if (received_bytes == 0)
    {
        return -1;
    }

    gdb_rx_count = received_bytes - 1;
    gdb_rx_last_pos = 1;
    return gdb_rx_buf[0];
}

#include "swdptap.h"

_Noreturn void gdb_main_1(void *pParams)
{
    struct target_controller *controller = &(gdb_controller[0]);
    gdb_main_init_target_controller(controller);

    platform_target_interface_non_iso_init(controller);

    //controller->connect_assert_srst = true;
    add_pid_to_list();


#if (USE_PIO == 0)
    gpio_init(TDI_PIN);
    gpio_set_dir(TDI_PIN, GPIO_OUT);
    gpio_put(TDI_PIN, 0);

    gpio_init(TCK_PIN);
    gpio_set_dir(TCK_PIN, GPIO_OUT);
    gpio_put(TCK_PIN, 0);

    gpio_init(TDO_PIN);
    gpio_set_dir(TDO_PIN, GPIO_IN);

    gpio_init(TMS_PIN);
    gpio_set_dir(TMS_PIN, GPIO_IN);

    gpio_init(TMS_DIR_PIN);
    gpio_set_dir(TMS_DIR_PIN, GPIO_OUT);
    gpio_put(TMS_DIR_PIN, 0);
#endif

    while (true)
    {
        volatile struct exception e;
        TRY_CATCH(e, EXCEPTION_ALL)
        {
                gdb_main_loop(controller, false);
        }
        if (e.type)
        {
            gdb_putpacketz("EFF");
            target_list_free(&(controller->target_list));
        }
    }
}

void vApplicationTickHook( void )
{
    /*static uint32_t tick_cnt = 0;
    tick_cnt++;
    static uint16_t state = 0;
    if (tick_cnt >= 500)
    {
        tick_cnt = 0;
        if (state)
        {
            state = 0;
        }
        else
        {
            state = 0xFFFF;
        }

        gpio_init(SHIFT_LED_CLK_GPIO);
        gpio_set_dir(SHIFT_LED_CLK_GPIO, GPIO_OUT);
        gpio_put(SHIFT_LED_CLK_GPIO, 0);

        gpio_init(SHIFT_LED_DATA_GPIO);
        gpio_set_dir(SHIFT_LED_DATA_GPIO, GPIO_OUT);
        gpio_put(SHIFT_LED_DATA_GPIO, 0);

        gpio_init(SHIFT_LED_LATCH_GPIO);
        gpio_set_dir(SHIFT_LED_LATCH_GPIO, GPIO_OUT);
        gpio_put(SHIFT_LED_LATCH_GPIO, 0);

        gpio_init(SHIFT_LED_OE_GPIO);
        gpio_set_dir(SHIFT_LED_OE_GPIO, GPIO_OUT);
        gpio_put(SHIFT_LED_OE_GPIO, 0);

        gpio_put(SHIFT_LED_LATCH_GPIO, 0);
        Mux_ShiftDelay();

        for (uint8_t i = 0; i < MUX_SHIFT_GPIO_NUMBER; i++)
        {
            gpio_put(SHIFT_LED_CLK_GPIO, 0);
            gpio_put(SHIFT_LED_DATA_GPIO, (state >> i) & 0x01);
            Mux_ShiftDelay();

            gpio_put(SHIFT_LED_CLK_GPIO, 1);
            Mux_ShiftDelay();
        }
        gpio_put(SHIFT_LED_LATCH_GPIO, 1);
    }*/
}

void vApplicationIdleHook( void )
{
    //LogWarn("Idle");
}

static SemaphoreHandle_t usb_uart_mutex;

/*_Noreturn void usb_uart_rx_thread(void *pParams)
{
    while(1)
    {

    }
}*/

_Noreturn void usb_uart_rx_thread(void *pParams)
{
    StreamBufferHandle_t usb_uart_thread_rx_stream = *(StreamBufferHandle_t*)pParams;
    char data = 0;

    while(1)
    {
        const size_t received_bytes = xStreamBufferReceive(usb_uart_thread_rx_stream, &data, sizeof(data),pdMS_TO_TICKS(25));
        if (received_bytes > 0)
        {
            system_pin_set(PIN_TARGET_1_LED_SER);
            if (uart_is_enabled(uart1))
            {
                uart_putc(uart1, data);
            }
            if (xStreamBufferBytesAvailable(usb_uart_thread_rx_stream) < sizeof(data))
            {
                system_pin_clear(PIN_TARGET_1_LED_SER);
            }
        }
        else
        {
            system_pin_clear(PIN_TARGET_1_LED_SER);
        }
    }
}

int dma_chan1 = -1;
int dma_chan2 = -1;

__attribute__((aligned(256)))
uint16_t uart1_data1_buf[2][128] = { 0 };

bool dma_occured = false;

bool timer_callback(repeating_timer_t *rt)
{
    static bool dma_flag_copy = false;
    if (dma_occured)
    {
        /*if (dma_flag_copy == false)
        {
            dma_flag_copy = true;
        }*/
        dma_occured = false;
        return true;
    }
    else
    {
        //if (dma_flag_copy)
        {
            dma_flag_copy = false;

            hw_write_masked(&dma_channel_hw_addr(dma_chan1)->ctrl_trig, 0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                            DMA_CH0_CTRL_TRIG_EN_BITS);

            hw_write_masked(&dma_channel_hw_addr(dma_chan2)->ctrl_trig, 0 << DMA_CH1_CTRL_TRIG_EN_MSB,
                            DMA_CH1_CTRL_TRIG_EN_BITS);

            uint32_t transfer_count = dma_channel_hw_addr(dma_chan1)->transfer_count;
            size_t written_bytes = 0;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;

            const uint32_t buf_size = 128 - transfer_count;
            uint32_t byte_count = 0;
            uint32_t total_bytes = 0;

            if ((transfer_count < 128) && (transfer_count > 0))
            {
                if (dma_channel_hw_addr(dma_chan2)->transfer_count == 0)
                {
                    uart1_data1_buf[0][128 - transfer_count - 1] |= 0x8000;
                }
                written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, uart1_data1_buf[0], (128 - transfer_count)*2, &xHigherPriorityTaskWoken);
                if (written_bytes != buf_size)
                {
                    byte_count = 0;
                }
                else
                {
                    total_bytes += written_bytes;
                }
            }

            transfer_count = dma_channel_hw_addr(dma_chan2)->transfer_count;
            if ((transfer_count < 128) && (transfer_count > 0))
            {
                if (dma_channel_hw_addr(dma_chan1)->transfer_count == 0)
                {
                    uart1_data1_buf[1][128 - transfer_count - 1] |= 0x8000;
                }
                written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, uart1_data1_buf[1], (128 - transfer_count)*2, &xHigherPriorityTaskWoken);
                if (written_bytes != buf_size)
                {
                    byte_count = 0;
                }
                else
                {
                    total_bytes += written_bytes;
                }
            }

            dma_channel_set_write_addr(dma_chan1, uart1_data1_buf[0], false);
            dma_channel_set_trans_count(dma_chan1, 128, false);

            dma_channel_set_write_addr(dma_chan2, uart1_data1_buf[1], false);
            dma_channel_set_trans_count(dma_chan2, 128, false);

            dma_channel_set_irq0_enabled(dma_chan1, false);
            dma_channel_set_irq0_enabled(dma_chan2, false);

            dma_channel_abort(dma_chan1);
            dma_channel_abort(dma_chan2);

            hw_write_masked(&uart_get_hw(uart1)->dmacr, 0 << UART_UARTDMACR_RXDMAE_MSB,
                            UART_UARTDMACR_RXDMAE_BITS);

            uart_set_irq_enables(uart1, true, false);

            return false;
        }
        return true;
    }
}

repeating_timer_t timer_test;

void dma1_handler();

void on_uart_rx()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint16_t buf[8] = { 0 };
    const uint8_t buf_length = sizeof(buf)/sizeof(buf[0]);
    const uint8_t buf_size = sizeof(buf);
    uint8_t byte_count = 0;
    size_t written_bytes = 0;
    static uint32_t total_bytes = 0;

    system_pin_clear(PIN_TARGET_1_LED_SER);

    uart_set_irq_enables(uart1, false, false);

    memset(&timer_test, 0, sizeof(timer_test));
    add_repeating_timer_ms(50, timer_callback, NULL, &timer_test);

    //uart1_data1_buf[0][0] = uart_getc(uart1);

    hw_write_masked(&uart_get_hw(uart1)->dmacr, 1 << UART_UARTDMACR_RXDMAE_MSB,
                    UART_UARTDMACR_RXDMAE_BITS);

    /*hw_write_masked(&dma_channel_hw_addr(dma_chan1)->al1_ctrl, 1 << DMA_CH0_CTRL_TRIG_EN_MSB,
                    DMA_CH0_CTRL_TRIG_EN_BITS);
    hw_write_masked(&dma_channel_hw_addr(dma_chan2)->ctrl_trig, 1 << DMA_CH1_CTRL_TRIG_EN_MSB,
                    DMA_CH1_CTRL_TRIG_EN_BITS);

    dma_channel_set_trans_count(dma_chan1, 127, false);
    dma_channel_set_trans_count(dma_chan2, 128, false);

    dma_channel_set_write_addr(dma_chan2, uart1_data1_buf[1], false);
    dma_channel_set_write_addr(dma_chan1, uart1_data1_buf[0] + 2, false);*/

    dma_channel_config dma_config1 = dma_channel_get_default_config(dma_chan1);
    channel_config_set_transfer_data_size(&dma_config1, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_config1, false);
    channel_config_set_write_increment(&dma_config1, true);
    //
    // channel_config_set_ring(&dma_config1, true, CAPTURE_RING_BITS);
    channel_config_set_chain_to(&dma_config1, dma_chan2);
    channel_config_set_dreq(&dma_config1, DREQ_UART1_RX);
    // When done, start the other channel.
    //channel_config_set_chain_to(&dma_config1, dma_chan2);
    // Using interrupt channel 0
    dma_channel_acknowledge_irq0(dma_chan1);
    dma_channel_acknowledge_irq0(dma_chan2);
    dma_channel_set_irq0_enabled(dma_chan1, true);
    // Set IRQ handler.
    irq_set_exclusive_handler(DMA_IRQ_0, dma1_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    dma_channel_configure(dma_chan1, &dma_config1,
                          uart1_data1_buf[0],   // dst
                          &(uart_get_hw(uart1)->dr),  // src
                          128,  // transfer count
                          true            // start immediately
    );

    dma_channel_config dma_config2 = dma_channel_get_default_config(dma_chan2);
    channel_config_set_transfer_data_size(&dma_config2, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_config2, false);
    channel_config_set_write_increment(&dma_config2, true);
    //
    // channel_config_set_ring(&dma_config1, true, CAPTURE_RING_BITS);
    channel_config_set_chain_to(&dma_config2, dma_chan1);
    channel_config_set_dreq(&dma_config2, DREQ_UART1_RX);
    // When done, start the other channel.
    //channel_config_set_chain_to(&dma_config1, dma_chan2);
    // Using interrupt channel 0
    dma_channel_set_irq0_enabled(dma_chan2, true);
    // Set IRQ handler.
    dma_channel_configure(dma_chan2, &dma_config2,
                          uart1_data1_buf[1],   // dst
                          &(uart_get_hw(uart1)->dr),  // src
                          128,  // transfer count
                          false            // start immediately
    );

    //dma_channel_hw_addr(dma_chan1)->ctrl_trig = dma_channel_hw_addr(dma_chan1)->ctrl_trig;
    //dma_channel_set_write_addr(dma_chan1, uart1_data1_buf[0] + 2, true);

    /*hw_write_masked(&dma_channel_hw_addr(dma_chan1)->ctrl_trig, 1 << DMA_CH0_CTRL_TRIG_EN_MSB,
                    DMA_CH0_CTRL_TRIG_EN_BITS);*/

    /*hw_write_masked(&dma_channel_hw_addr(dma_chan2)->ctrl_trig, 1 << DMA_CH1_CTRL_TRIG_EN_MSB,
                    DMA_CH1_CTRL_TRIG_EN_BITS);*/



    /*uint32_t transfer_count = dma_channel_hw_addr(dma_chan1)->transfer_count;

    if (transfer_count < 128)
    {
        written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, uart1_data1_buf[0], 128 - transfer_count, &xHigherPriorityTaskWoken);
        if (written_bytes != buf_size)
        {
            byte_count = 0;
        }
        else
        {
            total_bytes += written_bytes;
        }
    }
    transfer_count = dma_channel_hw_addr(dma_chan2)->transfer_count;

    if (transfer_count < 128)
    {
        written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, uart1_data1_buf[1], 128 - transfer_count, &xHigherPriorityTaskWoken);
        if (written_bytes != buf_size)
        {
            byte_count = 0;
        }
        else
        {
            total_bytes += written_bytes;
        }
    }*/


    /*while (uart_is_readable(uart1))
    {
        buf[byte_count++] = uart_getc(uart1);
        if (byte_count == buf_length) {
            if (!uart_is_readable(uart1)) {
                buf[byte_count - 1] |= 0x8000;
            }
            byte_count = 0;
            written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, buf, buf_size, &xHigherPriorityTaskWoken);
            if (written_bytes != buf_size)
            {
                byte_count = 0;
            }
            else
            {
                total_bytes += written_bytes;
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    if (byte_count)
    {
        buf[byte_count - 1] |= 0x8000;
        written_bytes = xStreamBufferSendFromISR(usb_uart_tx_stream, buf, byte_count * sizeof(buf[0]), &xHigherPriorityTaskWoken);
        if (written_bytes != byte_count * sizeof(buf[0]))
        {
            byte_count = 0;
        }
        else
        {
            total_bytes += written_bytes;
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }*/
}

uint32_t current_write_buffer_index = 0;

/*_Noreturn void usb_uart_line_coding_thread(void *pParams)
{
    StreamBufferHandle_t usb_uart_thread_line_coding_stream = *(StreamBufferHandle_t*)pParams;
    cdc_line_coding_t line_coding;

    while(1)
    {
        const size_t received_bytes = xStreamBufferReceive(usb_uart_thread_line_coding_stream, &line_coding, sizeof(line_coding), SYSTEM_WAIT_FOREVER);
        if (received_bytes == sizeof(line_coding))
        {
            if (dma_chan1 != -1)
            {
                //channel_config_set_enable()
                dma_channel_unclaim(dma_chan1);
            }
            else
            {
                //add_repeating_timer_ms(-10, timer_callback, NULL, &timer_test);
            }

            if (dma_chan2 != -1)
            {
                dma_channel_unclaim(dma_chan2);
            }

            dma_chan1 = dma_claim_unused_channel(true);
            dma_chan2 = dma_claim_unused_channel(true);

            // Chan 1
            {
                dma_channel_config dma_config1 = dma_channel_get_default_config(dma_chan1);
                channel_config_set_transfer_data_size(&dma_config1, DMA_SIZE_16);
                channel_config_set_read_increment(&dma_config1, false);
                channel_config_set_write_increment(&dma_config1, true);
                //
                // channel_config_set_ring(&dma_config1, true, CAPTURE_RING_BITS);
                channel_config_set_chain_to(&dma_config1, dma_chan2);
                channel_config_set_dreq(&dma_config1, DREQ_UART1_RX);
                // When done, start the other channel.
                //channel_config_set_chain_to(&dma_config1, dma_chan2);
                // Using interrupt channel 0
                dma_channel_set_irq0_enabled(dma_chan1, true);
                // Set IRQ handler.
                irq_set_exclusive_handler(DMA_IRQ_0, dma1_handler);
                irq_set_enabled(DMA_IRQ_0, true);
                dma_channel_configure(dma_chan1, &dma_config1,
                                      uart1_data1_buf[0],   // dst
                                      &(uart_get_hw(uart1)->dr),  // src
                                      128,  // transfer count
                                      false            // start immediately
                );

                dma_channel_config dma_config2 = dma_channel_get_default_config(dma_chan2);
                channel_config_set_transfer_data_size(&dma_config2, DMA_SIZE_16);
                channel_config_set_read_increment(&dma_config2, false);
                channel_config_set_write_increment(&dma_config2, true);
                //
                // channel_config_set_ring(&dma_config1, true, CAPTURE_RING_BITS);
                channel_config_set_chain_to(&dma_config2, dma_chan1);
                channel_config_set_dreq(&dma_config2, DREQ_UART1_RX);
                // When done, start the other channel.
                //channel_config_set_chain_to(&dma_config1, dma_chan2);
                // Using interrupt channel 0
                dma_channel_set_irq0_enabled(dma_chan2, true);
                // Set IRQ handler.
                dma_channel_configure(dma_chan2, &dma_config2,
                                      uart1_data1_buf[1],   // dst
                                      &(uart_get_hw(uart1)->dr),  // src
                                      128,  // transfer count
                                      false            // start immediately
                );
            }*/

            // Chan 2
            //{
               /* dma_channel_config dma_config2 = dma_channel_get_default_config(dma_chan2);
                channel_config_set_transfer_data_size(&dma_config2, DMA_SIZE_16);
                channel_config_set_read_increment(&dma_config2, false);
                channel_config_set_write_increment(&dma_config2, true);
                channel_config_set_ring(&dma_config2, true, CAPTURE_RING_BITS);
                channel_config_set_dreq(&dma_config2, DREQ_ADC);
                // When done, start the other channel.
                channel_config_set_chain_to(&dma_config2, dma_chan1);
                dma_channel_set_irq1_enabled(dma_chan2, true);
                irq_set_exclusive_handler(DMA_IRQ_1, dma_handler2);
                irq_set_enabled(DMA_IRQ_1, true);
                dma_channel_configure(dma_chan2, &dma_config2,
                                      capture_buf2,   // dst
                                      &adc_hw->fifo,  // src
                                      CAPTURE_DEPTH,  // transfer count
                                      false           // Do not start immediately*/

            /*portENTER_CRITICAL();
            uart_deinit(uart1);
            portEXIT_CRITICAL();

            uart_init(uart1, line_coding.bit_rate);
            uart_set_hw_flow(uart1, false, false);
            uart_set_format(uart1, line_coding.data_bits, line_coding.stop_bits + 1, line_coding.parity);

            irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
            irq_set_enabled(UART1_IRQ, true);

            //uart_set_fifo_enabled(uart1, false);
            uart_set_irq_enables(uart1, true, false);

            //hw_write_masked(&uart_get_hw(uart1)->imsc, 1 << UART_UARTIMSC_RTIM_MSB,
            //                UART_UARTIMSC_RTIM_BITS);
            hw_write_masked(&uart_get_hw(uart1)->dmacr, 0 << UART_UARTDMACR_RXDMAE_MSB,
                            UART_UARTDMACR_RXDMAE_BITS);

            hw_write_masked(&dma_channel_hw_addr(dma_chan1)->ctrl_trig, 0 << DMA_CH0_CTRL_TRIG_EN_MSB,
                            DMA_CH0_CTRL_TRIG_EN_BITS);

            hw_write_masked(&dma_channel_hw_addr(dma_chan2)->ctrl_trig, 0 << DMA_CH1_CTRL_TRIG_EN_MSB,
                            DMA_CH1_CTRL_TRIG_EN_BITS);

        }
    }
}*/

void main(void)
{
    traceSTART();

    board_init();

    system_pins_init();

    System_Init();

    MemManager_Init();

    device_settings_init();

    led_init();

    //device_make_usb_descriptor(2);

    BaseType_t status;
    /*status = xTaskCreate(LedTaskThread,
                                    "LEDTask",
                                    128,
                                    NULL,
                                    2,
                                    &LedTask);*/


    TaskHandle_t TestLedTask;
    /*status = xTaskCreate(TestLedTaskThread,
                                    "LEDTestTask",
                            configMINIMAL_STACK_SIZE,
                                    NULL,
                                    1,
                                    &TestLedTask);*/

    TaskHandle_t LogTask;
    /*status = xTaskCreate(LogTaskThread,
                         "LogTask",
                         512,
                         NULL,
                         1,
                         &LogTask);*/

    /*TaskHandle_t EthTask;
    status = xTaskCreate(EthTaskThread,
                         "EthTask",
                         configMINIMAL_STACK_SIZE*2,
                         NULL,
                         SYSTEM_PRIORITY_LOWEST,
                         &EthTask);*/

    w5500_task_init();

    TaskHandle_t gdb_main_thread_1;
    status = xTaskCreate(gdb_main_1,
                         "gdb_main_thread_1",
                         5192,
                         NULL,
                         SYSTEM_PRIORITY_LOW,
                         &gdb_main_thread_1);


    //rx_queue = xQueueCreate(512, sizeof(char));
    //tx_queue = xQueueCreate(512, sizeof(uint16_t));

    //cli_rx_queue = xQueueCreate(512, sizeof(char));
    //cli_tx_queue = xQueueCreate(512, sizeof(uint16_t));

    rx_stream = xStreamBufferCreate(4096, 1);
    tx_stream = xStreamBufferCreate(512, 256);
    line_coding_stream = xStreamBufferCreate(sizeof(cdc_line_coding_t)*5, sizeof(cdc_line_coding_t));

    tcp_server_config_t *p_tcp_server_config = MemManager_Alloc(sizeof(tcp_server_config_t));
    p_tcp_server_config->port = 10032;
    p_tcp_server_config->send_eot = true;
    p_tcp_server_config->rx_stream = rx_stream;
    p_tcp_server_config->tx_stream = tx_stream;
    p_tcp_server_config->socket_number = W5500_SOCK_UNUSED;

    tcp_server_add(p_tcp_server_config);

    // And set up and enable the interrupt handlers
    //irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    //irq_set_enabled(UART1_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    //uart_set_irq_enables(uart1, true, false);

    usb_task_init();

    //usb_cdc_task_init(0, "usb_gdb_rx_thread", "usb_gdb_tx_thread", rx_stream, tx_stream, line_coding_stream);

    usb_uart_init();
    //usb_uart_add_interface(0, 1);

    //usb_cdc_test_task_init(0, "usb_uart_rx_thread", "usb_uart_tx_thread", usb_uart_rx_stream, usb_uart_tx_stream);
    //usb_cdc_task_init(0, rx_queue, tx_queue);
    //usb_cdc_task_init(1, cli_rx_queue, cli_tx_queue);

    //vTaskCoreAffinitySet(LedTask, 0x01);
    //vTaskCoreAffinitySet(TestLedTask, 0x01);
    //vTaskCoreAffinitySet(LogTask, 0x01);
    //vTaskCoreAffinitySet(EthTask, 0x01);
    vTaskCoreAffinitySet(gdb_main_thread_1, 0x01);

    //uint8_t *pTest = (uint8_t*)MemManager_Alloc(256);
    //MemManager_Free(pTest);
    vTaskStartScheduler();
    //platform_init();
	//gdb_threads_start();
	//__disable_irq();
	//platform_cdc_start();
	//__enable_irq();

	while(1);
}

