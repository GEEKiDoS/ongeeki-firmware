#include "stdinclude.hpp"

#include <esp_private/usb_phy.h>
#include <tusb.h>
#include "desc.hpp"

/**
 * Note:
 * VID_0CA3&PID_0021&MI_00 is LED Board (COM3)
 * VID_0CA3&PID_0021&MI_02 is AIME Reader (COM1)
 * VID_0CA3&PID_0021&MI_04 is IO4 Board
 */

namespace usb {
    static usb_phy_handle_t phy_hdl;
    static TaskHandle_t s_tusb_tskh;
    static uint16_t desc_str_buffer[128];

    void init() {
        usb_phy_config_t phy_conf = {
                .controller = USB_PHY_CTRL_OTG,
                .target = USB_PHY_TARGET_INT,
                .otg_mode = USB_OTG_MODE_DEVICE,
                .otg_speed = USB_PHY_SPEED_FULL,
                .gpio_conf = nullptr,
        };

        usb_new_phy(&phy_conf, &phy_hdl);

        tusb_init();
        usb::hid::init();
        usb::serial::init();
    }
}

// region tinyusb callbacks
extern "C" {
    uint8_t const *tud_descriptor_device_cb() {
        return (uint8_t const *) &usb::desc_device;
    }

    uint8_t const *tud_descriptor_device_qualifier_cb() {
        return (uint8_t const *) &usb::desc_device_qualifier;
    }

    uint8_t const *tud_descriptor_other_speed_configuration_cb(uint8_t) {
        // if link speed is high return fullspeed config, and vice versa
        return usb::desc_configuration;
    }

    uint8_t const *tud_descriptor_configuration_cb(uint8_t) {
        // if link speed is high return fullspeed config, and vice versa
        return usb::desc_configuration;
    }

    uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t) {
        uint8_t chr_count;

        if (index == 0) {
            memcpy(&usb::desc_str_buffer[1], usb::string_desc_arr[0], 2);
            chr_count = 1;
        } else {
            // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
            // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

            if (index >= sizeof(usb::string_desc_arr) / sizeof(usb::string_desc_arr[0])) return nullptr;

            const char *str = usb::string_desc_arr[index];

            chr_count = strlen(str);

            // Convert ASCII strings into UTF-16
            for (uint8_t i = 0; i < chr_count; i++) {
                usb::desc_str_buffer[1 + i] = (uint8_t)str[i];
            }
        }

        // first byte is length (including header), second byte is string type
        usb::desc_str_buffer[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return usb::desc_str_buffer;
    }

    uint8_t const * tud_hid_descriptor_report_cb(uint8_t) {
        return usb::desc_hid_report;
    }

    uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t*, uint16_t) {
        return 0;
    }

    void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t buf_size) {
        auto data = reinterpret_cast<const usb::io4::input_t *>(buffer);
        if(report_id == 0x10) {
            usb::hid::process_data(data);
        }
    }
}
// end region
