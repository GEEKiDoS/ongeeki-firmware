#pragma once

namespace usb {
    const uint16_t vid = 0x0ca3;
    const uint16_t pid = 0x0021;

    tusb_desc_device_t const desc_device = {
            .bLength            = sizeof(tusb_desc_device_t),
            .bDescriptorType    = TUSB_DESC_DEVICE,
            .bcdUSB             = 0x200,

            // Use Interface Association Descriptor (IAD) for CDC
            // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
            .bDeviceClass       = TUSB_CLASS_MISC,
            .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
            .bDeviceProtocol    = MISC_PROTOCOL_IAD,
            .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

            .idVendor           = vid,
            .idProduct          = pid,
            .bcdDevice          = 0x0100,

            .iManufacturer      = 0x01,
            .iProduct           = 0x02,
            .iSerialNumber      = 0x03,

            .bNumConfigurations = 0x01
    };

    uint8_t const desc_hid_report[] = {
            0x05, 0x01,                     // Usage Page (Generic Desktop Ctrls)
            0x09, 0x04,                     // Usage (Joystick)
            0xA1, 0x01,                     // Collection (Application)
            0x85, 0x01,                     //   Report ID (1)
            0x09, 0x01,                     //   Usage (Pointer)
            0xA1, 0x00,                     //   Collection (Physical)
            0x09, 0x30,                     //     Usage (X)
            0x09, 0x31,                     //     Usage (Y)
            0x09, 0x30,                     //     Usage (X)
            0x09, 0x31,                     //     Usage (Y)
            0x09, 0x30,                     //     Usage (X)
            0x09, 0x31,                     //     Usage (Y)
            0x09, 0x30,                     //     Usage (X)
            0x09, 0x31,                     //     Usage (Y)
            0x09, 0x33,                     //     Usage (Rx)
            0x09, 0x34,                     //     Usage (Ry)
            0x09, 0x33,                     //     Usage (Rx)
            0x09, 0x34,                     //     Usage (Ry)
            0x09, 0x36,                     //     Usage (Slider)
            0x09, 0x36,                     //     Usage (Slider)
            0x15, 0x00,                     //     Logical Minimum (0)
            0x27, 0xFF, 0xFF, 0x00, 0x00,   //     Logical Maximum (65534)
            0x35, 0x00,                     //     Physical Minimum (0)
            0x47, 0xFF, 0xFF, 0x00, 0x00,   //     Physical Maximum (65534)
            0x95, 0x0E,                     //     Report Count (14)
            0x75, 0x10,                     //     Report Size (16)
            0x81, 0x02,                     //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0xC0,                           //   End Collection
            0x05, 0x02,                     //   Usage Page (Sim Ctrls)
            0x05, 0x09,                     //   Usage Page (Button)
            0x19, 0x01,                     //   Usage Minimum (0x01)
            0x29, 0x30,                     //   Usage Maximum (0x30)
            0x15, 0x00,                     //   Logical Minimum (0)
            0x25, 0x01,                     //   Logical Maximum (1)
            0x45, 0x01,                     //   Physical Maximum (1)
            0x75, 0x01,                     //   Report Size (1)
            0x95, 0x30,                     //   Report Count (48)
            0x81, 0x02,                     //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x09, 0x00,                     //   Usage (0x00)
            0x75, 0x08,                     //   Report Size (8)
            0x95, 0x1D,                     //   Report Count (29)
            0x81, 0x01,                     //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
            0x06, 0xA0, 0xFF,               //   Usage Page (Vendor Defined 0xFFA0)
            0x09, 0x00,                     //   Usage (0x00)
            0x85, 0x10,                     //   Report ID (16)
            0xA1, 0x01,                     //   Collection (Application)
            0x09, 0x00,                     //     Usage (0x00)
            0x15, 0x00,                     //     Logical Minimum (0)
            0x26, 0xFF, 0x00,               //     Logical Maximum (255)
            0x75, 0x08,                     //     Report Size (8)
            0x95, 0x3F,                     //     Report Count (63)
            0x91, 0x02,                     //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
            0xC0,                           //   End Collection
            0xC0,                           // End Collection
    };

    enum
    {
        ITF_NUM_CDC_0 = 0,
        ITF_NUM_CDC_0_DATA,
        ITF_NUM_CDC_1,
        ITF_NUM_CDC_1_DATA,
        ITF_NUM_HID,
        ITF_NUM_TOTAL
    };

    const uint16_t descriptor_length = (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN + CFG_TUD_HID * TUD_HID_INOUT_DESC_LEN);

    uint8_t const desc_configuration[] = {
            TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, descriptor_length, 0x00, 100),
            TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 6, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), 0x01,0x81, CFG_TUD_HID_EP_BUFSIZE, 1),
            TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, 0x82, 8, 0x03, 0x83, 64),
            TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 5, 0x84, 8, 0x05, 0x85, 64),
    };

    tusb_desc_device_qualifier_t const desc_device_qualifier = {
            .bLength            = sizeof(tusb_desc_device_t),
            .bDescriptorType    = TUSB_DESC_DEVICE,
            .bcdUSB             = 0x200,

            .bDeviceClass       = TUSB_CLASS_MISC,
            .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
            .bDeviceProtocol    = MISC_PROTOCOL_IAD,

            .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
            .bNumConfigurations = 0x01,
            .bReserved          = 0x00
    };

    char const* string_desc_arr[] = {
            // 0: is supported language is English (0x0409)
            (const char[]) { 0x09, 0x04 },
            // 1: Manufacturer
            "SEGA",
            // 2: Product
            "ONGEEKi I/O Board Type 4",
            // 3: Serials
            "00015257",
            // 4: CDC1, Aime Reader
            "AIME READER",
            // 5: CDC2, Led Board
            "LED BOARD",
            // 6: IO4 Board Name
            "I/O CONTROL BD;15257;01;90;1831;6679A;00;GOUT=14_ADIN=8,E_ROTIN=4_COININ=2_SWIN=2,E_UQ1=41,6;",
    };
}
