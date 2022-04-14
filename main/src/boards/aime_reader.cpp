#include "stdinclude.hpp"
#include "aime_cmd.hpp"

namespace aime_reader {
    const usb::serial::stream *stream;

    void init(const usb::serial::stream *input) {
        stream = input;
    }

    void reset() {

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
            }

            checksum += byte;
        }
    }
}
