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
                sg_nfc_cmd_reset();
                break;
            case kNFCCMD_GET_FW_VERSION:
                sg_nfc_cmd_get_fw_version();
                break;
            case kNFCCMD_GET_HW_VERSION:
                sg_nfc_cmd_get_hw_version();
                break;
            case kNFCCMD_CARD_DETECT:
                sg_nfc_cmd_poll();
                break;
            case kNFCCMD_MIFARE_READ:
                sg_nfc_cmd_mifare_read_block();
                break;
            case kNFCCMD_NFC_THROUGH:
                sg_nfc_cmd_felica_encap();
                break;
            case kNFCCMD_MIFARE_AUTHORIZE_B:
                sg_nfc_cmd_aime_authenticate();
                break;
            case kNFCCMD_MIFARE_AUTHORIZE_A:
                sg_nfc_cmd_bana_authenticate();
                break;
            case kNFCCMD_CARD_SELECT:
                sg_nfc_cmd_mifare_select_tag();
                break;
            case kNFCCMD_MIFARE_KEY_SET_B:
                sg_nfc_cmd_mifare_set_key_aime();
                break;
            case kNFCCMD_MIFARE_KEY_SET_A:
                sg_nfc_cmd_mifare_set_key_bana();
                break;
            case kNFCCMD_START_POLLING:
                sg_nfc_cmd_radio_on();
                break;
            case kNFCCMD_STOP_POLLING:
                sg_nfc_cmd_radio_off();
                break;
            case kNFCCMD_EXT_TO_NORMAL_MODE:
                sg_led_cmd_reset();
                break;
            case kNFCCMD_EXT_BOARD_INFO:
                sg_led_cmd_get_info();
                break;
            case kNFCCMD_EXT_BOARD_LED_RGB:
                sg_led_cmd_set_color();
                break;
            default:
                sg_res_init();
                break;
        }

        if (res.cmd == 0) return;

        stream->write_head();
        for(auto i = 0; i < res.frame_len; i++) {
            stream->write(res.bytes[i]);
        }

        res.cmd = 0;
    }

    uint8_t in_size, checksum;

    void update() {
        while(stream->available()) {
            uint8_t byte;
            bool is_escaped = stream->read(byte);

            if(byte == 0xE0 && !is_escaped) {
                printf("Aime Reader: Recv Sync\n");
                in_size = 0;
                checksum = 0;

                continue;
            }

            req.bytes[in_size ++] = byte;

            if(in_size > 2 && in_size - 2 == req.frame_len && checksum == byte) {
                printf("Aime Reader: Recv %d bytes, checksum %d\n", in_size, checksum);
                on_packet();
            }

            checksum += byte;
        }
    }
}
