#include "tusb.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "stream_buffer.h"
#include "system_func.h"
#include "mem_manager.h"
#include "usb_cdc_task.h"

typedef struct
{
    uint8_t interface;
    QueueHandle_t rx_queue;
    QueueHandle_t tx_queue;
} cdc_config_t;

typedef struct
{
    uint8_t interface;
    StreamBufferHandle_t rx_stream;
    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t line_coding_stream;
} cdc_test_config_t;

static TaskHandle_t rx_thread_list[CFG_TUD_CDC] = { NULL };
static TaskHandle_t test_rx_thread_list[CFG_TUD_CDC] = { NULL };

void cdc_rx_task_thread(void* params);
void cdc_tx_task_thread(void* params);

void test_cdc_rx_task_thread(void* params);
void test_cdc_tx_task_thread(void* params);

void usb_cdc_test_task_init(const uint8_t interface, const char* rx_thread_name, const char* tx_thread_name, StreamBufferHandle_t rx_stream, StreamBufferHandle_t tx_stream, StreamBufferHandle_t line_coding_stream)
{
    static bool task_created = false;
    if ((interface < CFG_TUD_CDC) && (rx_thread_list[interface] == NULL))
    {
        cdc_test_config_t *config = MemManager_Alloc(sizeof(cdc_config_t));

        config->interface = interface;
        config->rx_stream = rx_stream;
        config->tx_stream = tx_stream;
        config->line_coding_stream = line_coding_stream;

        TaskHandle_t rx_task;
        BaseType_t status = xTaskCreate(test_cdc_rx_task_thread,
                                        rx_thread_name,
                                        configMINIMAL_STACK_SIZE*2,
                                        config,
                                        SYSTEM_PRIORITY_LOW,
                                        &rx_task);

        vTaskCoreAffinitySet(rx_task, 0x01);

        rx_thread_list[interface] = rx_task;

        TaskHandle_t tx_task;
        status = xTaskCreate(test_cdc_tx_task_thread,
                             tx_thread_name,
                             configMINIMAL_STACK_SIZE*2,
                             config,
                             SYSTEM_PRIORITY_LOW,
                             &tx_task);

        vTaskCoreAffinitySet(tx_task, 0x01);
    }
}

void usb_cdc_task_init(const uint8_t interface, const char* rx_thread_name, const char* tx_thread_name, QueueHandle_t rx_queue, QueueHandle_t tx_queue)
{
    if ((interface < CFG_TUD_CDC) && (rx_thread_list[interface] == NULL))
    {
        cdc_config_t *config = MemManager_Alloc(sizeof(cdc_config_t));

        config->interface = interface;
        config->rx_queue = rx_queue;
        config->tx_queue = tx_queue;

        //TaskHandle_t usb_cdc_rx_task;
        /*BaseType_t status = xTaskCreate(cdc_rx_task_thread,
                                        rx_thread_name,
                             1024,
                                        config,
                             SYSTEM_PRIORITY_HIGH,
                             &usb_cdc_rx_task);

        vTaskCoreAffinitySet(usb_cdc_rx_task, 0x01);*/

        //TaskHandle_t usb_cdc_tx_task;
        /*status = xTaskCreate(cdc_tx_task_thread,
                             tx_thread_name,
                            512,
                            config,
                                4,
                            &usb_cdc_tx_task);

        vTaskCoreAffinitySet(usb_cdc_tx_task, 0x01);*/
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

void tud_cdc_line_coding_cb(uint8_t interface, cdc_line_coding_t const* p_line_coding)
{
    if (interface < CFG_TUD_CDC)
    {
        xTaskNotify(rx_thread_list[interface], USB_CDC_DATA_EVENT_LINE_CODING_UPDATE, eSetBits);
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
                    //LogWarn("TX: snd");
                }
                data_length = 0;
            }
        }
    }
}

_Noreturn void test_cdc_rx_task_thread(void* params)
{
    cdc_test_config_t *config = (cdc_test_config_t*)(params);
    const uint8_t interface = config->interface;

    if (interface > CFG_TUD_CDC)
    {
        //TODO: assert
    }

    int32_t data;
    bool connectedFlag = false;
    uint32_t notificationValue = 0;

    LogWarn("USB CDC Rx Thread Start");
    while (1)
    {
        if (xTaskNotifyWait(0, USB_CDC_DATA_EVENT_ALL, &notificationValue, SYSTEM_WAIT_FOREVER) != pdFALSE)
        {
            if (notificationValue & USB_CDC_DATA_EVENT_LINE_CODING_UPDATE)
            {
                cdc_line_coding_t line_coding = {0};
                tud_cdc_n_get_line_coding(interface, &line_coding);

                if (xStreamBufferSpacesAvailable(config->line_coding_stream) < sizeof(line_coding)) {
                    xStreamBufferReset(config->line_coding_stream);
                }

                xStreamBufferSend(config->line_coding_stream, &line_coding, sizeof(line_coding),
                                  SYSTEM_WAIT_DONT_BLOCK);
            }
            if ((tud_cdc_n_connected(interface) == false) && (connectedFlag == true)) {
                connectedFlag = false;
                const char data_to_send = 0x04; // Send EOT
                xStreamBufferSend(config->rx_stream, &data_to_send, sizeof(data_to_send),
                                  SYSTEM_WAIT_DONT_BLOCK);
                //xQueueSend(config.rx_queue, &data_to_send, SYSTEM_WAIT_DONT_BLOCK);
                continue;
            }

            if ((tud_cdc_n_connected(interface) == true) && (connectedFlag == false)) {
                connectedFlag = true;
            }

            // Read all data
            while (tud_cdc_n_available(interface)) {
                data = tud_cdc_n_read_char(interface);
                if (data >= 0) {
                    const char data_to_send = (char) data;
                    //if (xQueueSend(config.rx_queue, &data_to_send, SYSTEM_WAIT_DONT_BLOCK) != pdTRUE)
                    if (xStreamBufferSend(config->rx_stream, &data_to_send, sizeof(data_to_send),
                                          SYSTEM_WAIT_DONT_BLOCK) != sizeof(data_to_send)) {
                        //TODO: looks like the rx queue is full
                    }
                }
            }
        }
    }
}

_Noreturn void test_cdc_tx_task_thread(void* params)
{
    cdc_test_config_t *config = (cdc_test_config_t*)(params);
    const uint8_t interface = config->interface;

    if (interface > CFG_TUD_CDC)
    {
        //TODO: assert
    }

    uint16_t data = 0;
    uint16_t data_length = 0;
    unsigned char cdc_buffer[64] = { 0 };
    uint16_t stream_buffer[64] = { 0 };

    LogWarn("USB CDC Tx Thread Start");
    while (1)
    {
        const size_t received_bytes = xStreamBufferReceive(config->tx_stream, stream_buffer, sizeof(stream_buffer),
                                                           pdMS_TO_TICKS(50));

        if (received_bytes > 0)
        {
            for (size_t i = 0; i < (received_bytes / sizeof(stream_buffer[0])); i++)
            {
                const uint32_t flush = (stream_buffer[i] & 0x8000);
                const unsigned char data_to_write = (stream_buffer[i] & 0xFF);
                cdc_buffer[data_length++] = data_to_write;

                if ((flush) || (data_length == sizeof(cdc_buffer)))
                {
                    if (tud_cdc_n_connected(interface) == true)
                    {
                        tud_cdc_n_write(interface, cdc_buffer, data_length);
                        tud_cdc_n_write_flush(interface);
                    }
                    data_length = 0;
                }
            }
            memset(stream_buffer, 0, sizeof(stream_buffer));
        }
    }
}