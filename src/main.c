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
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "atomic.h"
#include "mem_manager.h"
#include "device_config.h"
#include "wizchip_conf.h"
#include "usb_task.h"
#include "usb_cdc_task.h"

#define GDB_THREAD_NAME_LENGTH	(20)
/*#define GDB_STACK_SIZE			(THREAD_STACKSIZE_LARGE*2)
#define MORSE_STACK_SIZE		(THREAD_STACKSIZE_TINY)
#define GDB_THREAD_PRIO			(THREAD_PRIORITY_MAIN-2)
#define MORSE_THREAD_PRIO		(THREAD_PRIORITY_MIN-1)

static char gdb_stack[MAX_GDB_NUMBER][GDB_STACK_SIZE] = {0};
static char morse_stack[INTERFACE_NUMBER][THREAD_STACKSIZE_TINY] = {0};*/

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

#define MUX_GPIO_START  (0x80)

typedef enum
{
    MUX_TARGET1_LED_ACT = MUX_GPIO_START,
    MUX_TARGET1_LED_SER,
    MUX_TARGET1_LED_ERR,
    MUX_TARGET2_LED_ACT,
    MUX_TARGET2_LED_SER,
    MUX_TARGET2_LED_ERR,
    MUX_TARGET1_VREF_PWR_EN,
    MUX_TARGET2_VREF_PWR_EN,
    MUX_LED_ETH,
    MUX_LED_USB,
    MUX_LED_SYS,
    MUX_TARGET1_PWR_EN,
    MUX_TARGET1_RST
} Mux_GPIO_List_t;

#define SHIFT_LED_CLK_GPIO    (22)
#define SHIFT_LED_DATA_GPIO   (15)
#define SHIFT_LED_LATCH_GPIO  (21)
#define SHIFT_LED_OE_GPIO     (20)

TaskHandle_t LedTask;

#define MUX_GPIO_TASK_EVENT_UPDATE   (0x01)
#define MUX_GPIO_TASK_EVENT_TIMEOUT  (0x02)

static uint32_t mux_gpio_value = 0;

void mux_gpio_set(uint8_t pin, uint8_t value)
{
    if (pin < MUX_GPIO_START)
    {
        gpio_put(pin, value);
    }
    else
    {
        value &= ~MUX_GPIO_START;
        if (value)
        {
            Atomic_OR_u32(&mux_gpio_value, (1 << value));
        }
        else
        {
            Atomic_AND_u32(&mux_gpio_value, ~(1 << value));
        }

        if (System_IsInInterrupt())
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            BaseType_t status = xTaskNotifyFromISR(LedTask, MUX_GPIO_TASK_EVENT_UPDATE, eSetBits, &xHigherPriorityTaskWoken);

        }
        else
        {
            BaseType_t status = xTaskNotify(LedTask, MUX_GPIO_TASK_EVENT_UPDATE, eSetBits);
        }
    }
}

void mux_gpio_mask_set(uint32_t mask)
{
    Atomic_OR_u32(&mux_gpio_value, mask);

    //TODO: check interrupt context and check return value
    BaseType_t status = xTaskNotify(LedTask, MUX_GPIO_TASK_EVENT_UPDATE, eSetBits);
}

void mux_gpio_mask_clear(uint32_t mask)
{
    Atomic_AND_u32(&mux_gpio_value, ~mask);

    //TODO: check interrupt context and check return value
    BaseType_t status = xTaskNotify(LedTask, MUX_GPIO_TASK_EVENT_UPDATE, eSetBits);
}

#define MUX_ALARM_NUMBER        (0)
#define MUX_TIMER_IRQ           (TIMER_IRQ_0)
#define MUX_SHIFT_GPIO_NUMBER   (13)
#define MUX_TIMER_MAX_TIMEOUT   (5) //in ticks
#define MUX_TIMER_ALARM_TIMEOUT (5)

static void Mux_ShiftDelay(void)
{
    for (uint32_t i = 0; i < 64; i++)
    {
        portNOP();
    }
}

void LedTaskThread (void *pParams)
{
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

    while (1)
    {
        const uint32_t gpio_state = mux_gpio_value;
        gpio_put(SHIFT_LED_LATCH_GPIO, 0);
        Mux_ShiftDelay();

        for (uint8_t i = 0; i < MUX_SHIFT_GPIO_NUMBER; i++)
        {
            gpio_put(SHIFT_LED_CLK_GPIO, 0);
            gpio_put(SHIFT_LED_DATA_GPIO, (gpio_state >> i) & 0x01);
            Mux_ShiftDelay();

            gpio_put(SHIFT_LED_CLK_GPIO, 1);
            Mux_ShiftDelay();
        }
        gpio_put(SHIFT_LED_LATCH_GPIO, 1);
        xTaskNotifyWait(0, MUX_GPIO_TASK_EVENT_UPDATE | MUX_GPIO_TASK_EVENT_TIMEOUT, NULL, pdMS_TO_TICKS(1000));
    }
}



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

void TestLedTaskThread (void *pParams)
{
    while(1)
    {
        //LogWarn("Led Task");
        //mux_gpio_mask_set(0xFFFF);
        //vTaskDelay(500);
        mux_gpio_mask_clear(0x0000);
        vTaskDelay(500);
    }
}

void LogTaskThread(void *pParams)
{
    char *message = NULL;

    while(1)
    {
        message = System_ReceiveLog();
        if (message != NULL)
        {
            printf("%s", message);
            MemManager_Free(message);
        }
    }
}

void wizchip_select(void)
{
    gpio_init(1);
    gpio_set_dir(1, GPIO_OUT);
    gpio_put(1, 0);
}

void wizchip_deselect(void)
{
    gpio_init(1);
    gpio_set_dir(1, GPIO_OUT);
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

static SemaphoreHandle_t ethMutex;

void wizchip_lock(void)
{
    xSemaphoreTake(ethMutex, SYSTEM_WAIT_FOREVER);
}

void wizchip_unlock(void)
{
    xSemaphoreGive(ethMutex);
}

void EthTaskThread(void *pParams)
{
    uint8_t memsize[2][8] = { { 2, 2, 2, 2, 2, 2, 2, 2 }, { 2, 2, 2, 2, 2, 2, 2, 2 } };

    wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},
            .ip = {192, 168, 1, 2},
            .sn = {255, 255, 255, 0},
            .gw = {192, 168, 1, 1},
            .dns = {0, 0, 0, 0},
            .dhcp = NETINFO_STATIC };

    ethMutex = xSemaphoreCreateMutex();

    spi_init(spi0, 33 * 1000 * 1000);
    gpio_set_function(0, GPIO_FUNC_SPI);
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(3, GPIO_FUNC_SPI);

    gpio_init(1);
    gpio_set_dir(1, GPIO_OUT);
    gpio_put(1, 1);

    gpio_init(4);
    gpio_set_dir(4, GPIO_OUT);
    gpio_put(4, 1);

    uint8_t reg = 0xFA;
    uint8_t mac[6] = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef};

    memcpy(gWIZNETINFO.mac, mac, sizeof(mac));

    vTaskDelay(1000);

    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
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

    while (1)
    {
        vTaskDelay(1000);
    }
}

#include "gdb_hostio.h"

struct target_controller gdb_controller[2];
QueueHandle_t rx_queue;
QueueHandle_t tx_queue;

void gdb_if_putchar(unsigned char c, int flush)
{
    const uint16_t data = flush ? (0x8000 | (uint8_t)c) : (uint8_t)c;
    //LogWarn("GDB putchar");
    xQueueSend(tx_queue, (void*)&data, SYSTEM_WAIT_DONT_BLOCK);
}

unsigned char gdb_if_getchar(void)
{
    unsigned char data;
    xQueueReceive(rx_queue, &data, SYSTEM_WAIT_FOREVER);
    return data;
}

unsigned char gdb_if_getchar_to(int timeout)
{
    uint8_t data = 0;

    if (xQueueReceive(rx_queue, &data, pdMS_TO_TICKS(timeout)) != pdTRUE)
    {
        return -1;
    }
    else
    {
        return data;
    }
}

void platform_srst_set_val(bool assert)
{
    if (assert)
    {
        Atomic_OR_u32(&mux_gpio_value, (1 << 12));
    }
    else
    {
        Atomic_AND_u32(&mux_gpio_value, ~(1 << 12));
    }
}

void gdb_main_1(void *pParams)
{
    struct target_controller *controller = &(gdb_controller[0]);
    gdb_main_init_target_controller(controller);
    controller->platform_srst_set_val = platform_srst_set_val;

    add_pid_to_list();

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
void main(void)
{
    board_init();

    System_Init();

    MemManager_Init();

    device_config_init();

    device_make_usb_descriptor(1);

    BaseType_t status;
    status = xTaskCreate(LedTaskThread,
                                    "LEDTask",
                                    128,
                                    NULL,
                                    2,
                                    &LedTask);

    TaskHandle_t TestLedTask;
    status = xTaskCreate(TestLedTaskThread,
                                    "LEDTestTask",
                            configMINIMAL_STACK_SIZE,
                                    NULL,
                                    1,
                                    &TestLedTask);

    TaskHandle_t LogTask;
    /*status = xTaskCreate(LogTaskThread,
                         "LogTask",
                         512,
                         NULL,
                         1,
                         &LogTask);*/

    TaskHandle_t EthTask;
    /*status = xTaskCreate(EthTaskThread,
                         "EthTask",
                         512,
                         NULL,
                         1,
                         &EthTask);*/

    TaskHandle_t gdb_main_thread_1;
    status = xTaskCreate(gdb_main_1,
                         "gdb_main_thread_1",
                         4096,
                         NULL,
                         3,
                         &gdb_main_thread_1);

    rx_queue = xQueueCreate(512, sizeof(char));
    tx_queue = xQueueCreate(512, sizeof(uint16_t));

    usb_task_init();

    usb_cdc_task_init(0, rx_queue, tx_queue);

    vTaskCoreAffinitySet(LedTask, 0x01);
    vTaskCoreAffinitySet(TestLedTask, 0x01);
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

