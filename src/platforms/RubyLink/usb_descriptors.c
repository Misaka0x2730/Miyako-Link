/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"

#define USB_VID   (0x1209)
#define USB_PID   (0x2730)
#define USB_BCD   (0x0200)

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
        .bLength            = sizeof(tusb_desc_device_t),
        .bDescriptorType    = TUSB_DESC_DEVICE,
        .bcdUSB             = USB_BCD,

        // Use Interface Association Descriptor (IAD) for CDC
        // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
        .bDeviceClass       = TUSB_CLASS_MISC,
        .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol    = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

        .idVendor           = USB_VID,
        .idProduct          = USB_PID,
        .bcdDevice          = USB_BCD,

        .iManufacturer      = 0x01,
        .iProduct           = 0x02,
        .iSerialNumber      = 0x03,

        .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum
{
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
        {
                (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
                "TinyUSB",                     // 1: Manufacturer
                "TinyUSB Device",              // 2: Product
                "123456",                      // 3: Serials, should use chip ID
                "TinyUSB CDC",                 // 4: CDC Interface
        };

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (!(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])))
        {
            return NULL;
        }

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31)
        {
            chr_count = 31;
        }

        // Convert ASCII string into UTF-16
        for (uint8_t i=0; i<chr_count; i++)
        {
            _desc_str[1+i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

    return _desc_str;
}

static uint8_t device_fs_full_descriptor[512] = { 0 };
static const uint8_t device_fs_endpoint_list[] = {
        0x81, 0x02, 0x82,
        0x83, 0x04, 0x84,
        0x85, 0x06, 0x86,
        0x87, 0x08, 0x88,

        0x89, 0x0A, 0x8A,
        0x8B, 0x0C, 0x8C,
        0x8D, 0x0E, 0x8E
};

extern uint8_t const desc_fs_configuration[];

void device_make_usb_descriptor(const uint8_t interface_count)
{
    if (interface_count > 7)
    {
        return;
    }

    const uint16_t total_len = TUD_CONFIG_DESC_LEN + (interface_count * TUD_CDC_DESC_LEN);
    const uint8_t device_fs_config_desc[] = {TUD_CONFIG_DESCRIPTOR(1, interface_count*2, 0, total_len, 0x00, 500)};

    memcpy(device_fs_full_descriptor, device_fs_config_desc, sizeof(device_fs_config_desc));

    /*device_fs_full_descriptor[2] = (uint8_t)(total_len & 0xFF);
    device_fs_full_descriptor[3] = (uint8_t)((total_len >> 8) & 0xFF);
    device_fs_full_descriptor[4] = interface_count * 2;*/

    for (uint8_t i = 0; i < interface_count; i++)
    {
        const uint16_t current_pos = TUD_CONFIG_DESC_LEN + (i * TUD_CDC_DESC_LEN);
        const uint8_t device_fs_interface_desc[] = {TUD_CDC_DESCRIPTOR(i*2, 4,
                                                                       device_fs_endpoint_list[i*3], 8,
                                                                       device_fs_endpoint_list[i*3+1],
                                                                       device_fs_endpoint_list[i*3+2], 64)};

        memcpy(device_fs_full_descriptor + current_pos, device_fs_interface_desc, sizeof(device_fs_interface_desc));
    }

    //memcpy(device_fs_full_descriptor, desc_fs_configuration, 141);
}

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // for multiple configurations

    return device_fs_full_descriptor;
}