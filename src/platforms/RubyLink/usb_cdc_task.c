#include "tusb.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "system_func.h"
#include "mem_manager.h"
#include "usb_cdc_task.h"

#define USB_CDC_DATA_EVENT_DATA_RX            (0x01)
#define USB_CDC_DATA_EVENT_CONNECTION_UPDATE  (0x02)

typedef struct
{
    uint8_t interface;
    QueueHandle_t rx_queue;
    QueueHandle_t tx_queue;
} cdc_config_t;

static TaskHandle_t rx_thread_list[CFG_TUD_CDC] = { NULL };

void cdc_rx_task_thread(void* params);
void cdc_tx_task_thread(void* params);

void usb_cdc_task_init(const uint8_t interface, QueueHandle_t rx_queue, QueueHandle_t tx_queue)
{
    if ((interface < CFG_TUD_CDC) && (rx_thread_list[interface] == NULL))
    {
        cdc_config_t *config = MemManager_Alloc(sizeof(cdc_config_t));

        config->interface = interface;
        config->rx_queue = rx_queue;
        config->tx_queue = tx_queue;

        char task_name[configMAX_TASK_NAME_LEN] = { 0 };
        snprintf(task_name, configMAX_TASK_NAME_LEN, "usb_cdc_rx_thread_%d", interface);

        TaskHandle_t usb_cdc_rx_task;
        BaseType_t status = xTaskCreate(cdc_rx_task_thread,
                                        task_name,
                             configMINIMAL_STACK_SIZE,
                                        config,
                             4,
                             &usb_cdc_rx_task);

        vTaskCoreAffinitySet(usb_cdc_rx_task, 0x01);

        rx_thread_list[interface] = usb_cdc_rx_task;

        snprintf(task_name, configMAX_TASK_NAME_LEN, "usb_cdc_tx_thread_%d", interface);

        TaskHandle_t usb_cdc_tx_task;
        status = xTaskCreate(cdc_tx_task_thread,
                                        task_name,
                                        512,
                                        config,
                                        4,
                                        &usb_cdc_tx_task);

        vTaskCoreAffinitySet(usb_cdc_tx_task, 0x01);
    }
}

void tud_cdc_rx_cb(uint8_t interface)
{
    if (interface < CFG_TUD_CDC)
    {
        xTaskNotify(rx_thread_list[interface], USB_CDC_DATA_EVENT_DATA_RX, eSetBits);
    }
}

void tud_cdc_line_state_cb(uint8_t interface, bool dtr, bool rts)
{
    (void) rts;
    (void) dtr;

    if (interface < CFG_TUD_CDC)
    {
        xTaskNotify(rx_thread_list[interface], USB_CDC_DATA_EVENT_CONNECTION_UPDATE, eSetBits);
    }
}

void cdc_rx_task_thread(void* params)
{
    const cdc_config_t config = *(cdc_config_t*)(params);
    const uint8_t interface = config.interface;

    if (interface > CFG_TUD_CDC)
    {
        //TODO: assert
        return;
    }

    int32_t data;
    bool connectedFlag = false;

    LogWarn("USB CDC Rx Thread Start");
    while (1)
    {
        xTaskNotifyWait(0, USB_CDC_DATA_EVENT_CONNECTION_UPDATE | USB_CDC_DATA_EVENT_DATA_RX, NULL, SYSTEM_WAIT_FOREVER);

        if ((tud_cdc_n_connected(interface) == false) && (connectedFlag == true))
        {
            connectedFlag = false;
            const char data_to_send = 0x04; // Send EOT
            xQueueSend(config.rx_queue, &data_to_send, SYSTEM_WAIT_DONT_BLOCK);
            continue;
        }

        if ((tud_cdc_n_connected(interface) == true) && (connectedFlag == false))
        {
            connectedFlag = true;
        }

        // Read all data
        while (tud_cdc_n_available(config.interface))
        {
            data = tud_cdc_n_read_char(config.interface);
            if (data >= 0)
            {
                const char data_to_send = (char)data;
                if (xQueueSend(config.rx_queue, &data_to_send, SYSTEM_WAIT_DONT_BLOCK) != pdTRUE)
                {
                    //TODO: looks like the rx queue is full
                }
            }
        }

       // vTaskDelay(10);
    }
}

void cdc_tx_task_thread(void* params)
{
    const cdc_config_t config = *(cdc_config_t*)(params);
    const uint8_t interface = config.interface;

    if (interface > CFG_TUD_CDC)
    {
        //TODO: assert
        return;
    }

    uint16_t data;
    uint16_t data_length = 0;
    unsigned char buffer[64] = { 0 };

    LogWarn("USB CDC Tx Thread Start");
    while (1)
    {
        BaseType_t  result = xQueueReceive(config.tx_queue, &data, SYSTEM_WAIT_FOREVER);
        //LogWarn("TX: snd");
        if (result == pdTRUE)
        {
            const bool flush = (data & 0x8000);
            const unsigned char data_to_write = (data & 0xFF);
            buffer[data_length++] = data_to_write;

            if ((flush) || (data_length == sizeof(buffer)))
            {
                if (tud_cdc_n_connected(interface) == true)
                {
                    tud_cdc_n_write(interface, buffer, data_length);
                    tud_cdc_n_write_flush(interface);
                    LogWarn("TX: snd");
                }
                data_length = 0;
            }
        }
    }
}