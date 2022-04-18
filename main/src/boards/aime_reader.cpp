#include "stdinclude.hpp"
#include "aime_cmd.hpp"

namespace aime_reader {
    const usb::serial::stream *stream;

    TaskHandle_t h_read_card;
    [[noreturn]] static void read_test_task(void *arg) {
        uint64_t idm;
        uint64_t pmm;
        uint16_t sys_code;

        while(true) {
            if (nfc.felica_Polling(0xFFFF, 0x01, (uint8_t *)&idm, (uint8_t *)&pmm, &sys_code, 200)) {
                printf("Felica card!\nIDm: %llx\nPMm: %llx\nSystem Code: %x\n", idm, pmm, sys_code);
            }

            vTaskDelay(2000 / portTICK_RATE_MS);
        }
    }

    void init(const usb::serial::stream *input) {
        stream = input;

        nfc.begin();
        auto fw_version = nfc.getFirmwareVersion();
        if(fw_version > 0) {
            printf("Found NFC Module, FW Version: %x\n", fw_version);
            nfc.setPassiveActivationRetries(0x10);
            nfc.SAMConfig();
        } else {
            printf("NFC Module not found, does it connected properly?\n");
        }

        // xTaskCreate(read_test_task, "test_nfc", 4096, nullptr, 5, &h_read_card);

        memset(&req, 0, sizeof(req.bytes));
        memset(&res, 0, sizeof(res.bytes));

    }

    void on_packet() {
        switch(req.cmd) {
            case kNFCCMD_TO_NORMAL_MODE:
                printf("Aime Board: To Normal Mode\n");
                sg_nfc_cmd_reset();
                break;
            case kNFCCMD_GET_FW_VERSION:
                printf("Aime Board: Get FW Version\n");
                sg_nfc_cmd_get_fw_version();
                break;
            case kNFCCMD_GET_HW_VERSION:
                printf("Aime Board: Get HW Version\n");
                sg_nfc_cmd_get_hw_version();
                break;
            case kNFCCMD_CARD_DETECT:
                printf("Aime Board: Card Detect\n");
                sg_nfc_cmd_poll();
                break;
            case kNFCCMD_MIFARE_READ:
                printf("Aime Board: Mifare Read\n");
                sg_nfc_cmd_mifare_read_block();
                break;
            case kNFCCMD_NFC_THROUGH:
                printf("Aime Board: NFC Though\n");
                sg_nfc_cmd_felica_encap();
                break;
            case kNFCCMD_MIFARE_AUTHORIZE_B:
                printf("Aime Board: Aime Auth\n");
                sg_nfc_cmd_aime_authenticate();
                break;
            case kNFCCMD_MIFARE_AUTHORIZE_A:
                printf("Aime Board: Bana Auth\n");
                sg_nfc_cmd_bana_authenticate();
                break;
            case kNFCCMD_CARD_SELECT:
                printf("Aime Board: Select Tag\n");
                sg_nfc_cmd_mifare_select_tag();
                break;
            case kNFCCMD_MIFARE_KEY_SET_B:
                printf("Aime Board: Set Aime Key\n");
                sg_nfc_cmd_mifare_set_key_aime();
                break;
            case kNFCCMD_MIFARE_KEY_SET_A:
                printf("Aime Board: Set Bana Key\n");
                sg_nfc_cmd_mifare_set_key_bana();
                break;
            case kNFCCMD_START_POLLING:
                printf("Aime Board: Start Polling\n");
                sg_nfc_cmd_radio_on();
                break;
            case kNFCCMD_STOP_POLLING:
                printf("Aime Board: Stop Polling\n");
                sg_nfc_cmd_radio_off();
                break;
            case kNFCCMD_EXT_TO_NORMAL_MODE:
                printf("Aime LED Board: To Normal Mode\n");
                sg_led_cmd_reset();
                break;
            case kNFCCMD_EXT_BOARD_INFO:
                printf("Aime LED Board: Get Board Info\n");
                sg_led_cmd_get_info();
                break;
            case kNFCCMD_EXT_BOARD_LED_RGB:
                printf("Aime LED Board: Sed LED RGB Color\n");
                sg_led_cmd_set_color();
                break;
            default:
                printf("Unknown Aime Command: 0x%x\n", req.cmd);
                sg_res_init();
                break;
        }

        if (res.cmd == 0) return;

        uint8_t checksum = 0;

        stream->write_head();
        for(auto i = 0; i < res.frame_len; i++) {
            checksum += res.bytes[i];
            stream->write(res.bytes[i]);
        }
        stream->write(checksum);
        stream->flush();

        res.cmd = 0;
    }

    uint8_t in_size, checksum;

    void update() {
        while(stream->available()) {
            uint8_t byte;
            bool is_escaped = stream->read(byte);

            if(byte == 0xE0 && !is_escaped) {
                // printf("Aime Reader: Recv Sync\n");
                in_size = 0;
                checksum = 0;

                continue;
            }

            req.bytes[in_size ++] = byte;

            if(in_size > 2 && in_size - 1 == req.frame_len && checksum == byte) {
                // printf("Aime Reader: Recv %d bytes, checksum %d\n", in_size, checksum);
                on_packet();
                // return;
            }

            checksum += byte;
        }
    }
}
