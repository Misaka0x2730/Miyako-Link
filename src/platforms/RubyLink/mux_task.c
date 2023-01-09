#include "FreeRTOS.h"
#include "atomic.h"
#include "system_func.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "mux_task.h"

static uint32_t mux_pins_value = 0;
static TaskHandle_t mux_task;

static void mux_task_thread(void *pParams);

static void mux_task_pins_init(void)
{
    gpio_init(PIN_MUX_CLK);
    gpio_set_dir(PIN_MUX_CLK, GPIO_OUT);
    gpio_put(PIN_MUX_CLK, 0);

    gpio_init(PIN_MUX_DATA);
    gpio_set_dir(PIN_MUX_DATA, GPIO_OUT);
    gpio_put(PIN_MUX_DATA, 0);

    gpio_init(PIN_MUX_LATCH);
    gpio_set_dir(PIN_MUX_LATCH, GPIO_OUT);
    gpio_put(PIN_MUX_LATCH, 0);

    gpio_init(PIN_MUX_OE);
    gpio_set_dir(PIN_MUX_OE, GPIO_OUT);
    gpio_put(PIN_MUX_OE, 0);
}

static void mux_task_pins_update(void)
{
    const uint32_t gpio_state = mux_pins_value;
    gpio_put(PIN_MUX_LATCH, 0);
    busy_wait_us(1);

    for (int i = PIN_MUX_COUNT - 1; i >= 0; i--)
    {
        gpio_put(PIN_MUX_CLK, 0);
        gpio_put(PIN_MUX_DATA, (gpio_state >> i) & 0x01);
        busy_wait_us(1);

        gpio_put(PIN_MUX_CLK, 1);
        busy_wait_us(1);
    }
    gpio_put(PIN_MUX_LATCH, 1);
}

void mux_task_init(void)
{
    mux_task_pins_init();

    BaseType_t status = xTaskCreate(mux_task_thread,
                         "mux_task",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         SYSTEM_PRIORITY_LOWEST,
                         &mux_task);

    vTaskCoreAffinitySet(mux_task, 0x01);
}

_Noreturn static void mux_task_thread(void *pParams)
{
    while (1)
    {
        mux_task_pins_update();
        xTaskNotifyWait(0, MUX_TASK_EVENT_UPDATE, NULL, pdMS_TO_TICKS(1000));
    }
}

void mux_pin_set(platform_pin_t pin, bool value)
{
    if (PIN_IS_MUX_PIN(pin))
    {
        pin &= ~MUX_PIN_BIT;

        if (pin >= PIN_MUX_COUNT)
        {
            //TODO: assert
            return;
        }

        const uint32_t mask = (1 << pin);
        if (value)
        {
            Atomic_OR_u32(&mux_pins_value, mask);
        }
        else
        {
            Atomic_AND_u32(&mux_pins_value, ~mask);
        }

        if (System_IsInInterrupt())
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            BaseType_t status = xTaskNotifyFromISR(mux_task, MUX_TASK_EVENT_UPDATE, eSetBits, &xHigherPriorityTaskWoken);
            if (status != pdTRUE)
            {
                //TODO: assert
            }
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
        else
        {
            BaseType_t status = xTaskNotify(mux_task, MUX_TASK_EVENT_UPDATE, eSetBits);
            if (status != pdTRUE)
            {
                //TODO: assert
            }
        }
    }
}

bool mux_pin_get(platform_pin_t pin)
{
    if (PIN_IS_MUX_PIN(pin))
    {
        pin &= ~MUX_PIN_BIT;

        if (pin < PIN_MUX_COUNT)
        {
            return (mux_pins_value & (1 << pin));
        }
    }

    return false;
}