#include "stdinclude.hpp"

#include <esp_private/usb_phy.h>
#include <tusb.h>
#include "desc.hpp"

#include "disk_img.h"

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

    [[noreturn]] static void task(void *arg) {
        while (true) {
            tud_task();
        }
    }

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

        xTaskCreate(task, "tinyusb_task", 4096, nullptr, 5, &s_tusb_tskh);
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
                usb::desc_str_buffer[1 + i] = (uint8_t) str[i];
            }
        }

        // first byte is length (including header), second byte is string type
        usb::desc_str_buffer[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return usb::desc_str_buffer;
    }

    uint8_t const *tud_hid_descriptor_report_cb(uint8_t) {
        return usb::desc_hid_report;
    }

    uint16_t tud_hid_get_report_cb(uint8_t, uint8_t, hid_report_type_t, uint8_t *, uint16_t) {
        return 0;
    }

    void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                               uint16_t buf_size) {
        auto data = reinterpret_cast<const usb::io4::input_t *>(buffer);
        if (report_id == 0x10) {
            usb::hid::process_data(data);
        }
    }

    // Invoked when device is mounted
    void tud_mount_cb(void) {
    }

    // Invoked when device is unmounted
    void tud_umount_cb(void) {
    }

    // Invoked when usb bus is suspended
    // remote_wakeup_en : if host allow us  to perform remote wakeup
    // Within 7ms, device must draw an average of current less than 2.5 mA from bus
    void tud_suspend_cb(bool remote_wakeup_en) {
        (void) remote_wakeup_en;
    }

    // Invoked when usb bus is resumed
    void tud_resume_cb(void) {
    }

    // Invoked when received SCSI_CMD_INQUIRY
    // Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
    void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[20], uint8_t product_rev[4]) {
        (void) lun;

        const char vid[] = "GEEKiDoS";
        const char pid[] = "ONGEEKi Driver Setup";
        const char rev[] = "1.0";

        memcpy(vendor_id, vid, strlen(vid));
        memcpy(product_id, pid, strlen(pid));
        memcpy(product_rev, rev, strlen(rev));
    }

    // Invoked when received Test Unit Ready command.
    // return true allowing host to read/write this LUN e.g SD card inserted
    bool tud_msc_test_unit_ready_cb(uint8_t lun) {
        (void) lun;

        return true; // RAM disk is always ready
    }

    // Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
    // Application update block count and block size
    void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {
        (void) lun;

        *block_count = DISK_BLOCK_NUM;
        *block_size = DISK_BLOCK_SIZE;
    }

    // Invoked when received Start Stop Unit command
    // - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
    // - Start = 1 : active mode, if load_eject = 1 : load disk storage
    bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) {
        (void) lun;
        (void) power_condition;

        if (load_eject) {
            if (start) {
                // load disk storage
            } else {
                // unload disk storage
            }
        }

        return true;
    }

    // Callback invoked when received READ10 command.
    // Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
    int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
        (void) lun;

        // out of ramdisk
        if (lba >= DISK_BLOCK_NUM) return -1;

        auto addr = disk_image + (lba * DISK_BLOCK_SIZE + offset);
        memcpy(buffer, addr, bufsize);

        return bufsize;
    }

    // Callback invoked when received WRITE10 command.
    // Process data in buffer to disk's storage and return number of written bytes
    int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
        (void) lun;

        // out of ramdisk
        if (lba >= DISK_BLOCK_NUM) return -1;

        (void) lba; (void) offset; (void) buffer;

        return bufsize;
    }

    // Callback invoked when received an SCSI command not in built-in list below
    // - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
    // - READ10 and WRITE10 has their own callbacks
    int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize) {
        // read10 & write10 has their own callback and MUST not be handled here

        void const *response = NULL;
        int32_t resplen = 0;

        // most scsi handled is input
        bool in_xfer = true;

        switch (scsi_cmd[0]) {
            default:
                // Set Sense = Invalid Command Operation
                tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

                // negative means error -> tinyusb could stall and/or response with failed status
                resplen = -1;
                break;
        }

        // return resplen must not larger than bufsize
        if (resplen > bufsize) resplen = bufsize;

        if (response && (resplen > 0)) {
            if (in_xfer) {
                memcpy(buffer, response, resplen);
            } else {
                // SCSI output
            }
        }

        return resplen;
    }
}
// end region
