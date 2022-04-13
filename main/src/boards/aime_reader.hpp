#pragma once

namespace aime_reader {
    namespace cAiMeNFCRW {
        enum eNFCCommandList : uint8_t {
            kNFCCMD_GET_FW_VERSION = 0x30,
            kNFCCMD_GET_HW_VERSION = 0x32,
            kNFCCMD_START_POLLING = 0x40,
            kNFCCMD_STOP_POLLING = 0x41,
            kNFCCMD_CARD_DETECT = 0x42,
            kNFCCMD_CARD_SELECT = 0x43,
            kNFCCMD_CARD_HALT = 0x44,
            kNFCCMD_MIFARE_KEY_SET_A = 0x50,
            kNFCCMD_MIFARE_AUTHORIZE_A = 0x51,
            kNFCCMD_MIFARE_READ = 0x52,
            kNFCCMD_MIFARE_WRITE = 0x53,
            kNFCCMD_MIFARE_KEY_SET_B = 0x54,
            kNFCCMD_MIFARE_AUTHORIZE_B = 0x55,
            kNFCCMD_TO_UPDATER_MODE = 0x60,
            kNFCCMD_SEND_HEX_DATA = 0x61,
            kNFCCMD_TO_NORMAL_MODE = 0x62,
            kNFCCMD_SEND_BINDATA_INIT = 0x63,
            kNFCCMD_SEND_BINDATA_EXEC = 0x64,
            kNFCCMD_FELICA_PUSH = 0x70,
            kNFCCMD_NFC_THROUGH = 0x71,
            kNFCCMD_EXT_BOARD_LED = 0x80,
            kNFCCMD_EXT_BOARD_LED_RGB = 0x81,
            kNFCCMD_EXT_BOARD_INFO = 0xF0,
            kNFCCMD_EXT_FIRM_SUM = 0xF2,
            kNFCCMD_EXT_SEND_HEX_DATA = 0xF3,
            kNFCCMD_EXT_TO_BOOT_MODE = 0xF4,
            kNFCCMD_EXT_TO_NORMAL_MODE = 0xF5,
        };

        union sNfcCommand {
            struct {
                uint8_t nSize;
                uint8_t idReader;
                uint8_t idPacketSequence;
                eNFCCommandList codeCommand;
                uint8_t nDataSize;
                union {
                    uint8_t data[124];
                };
            };
            uint8_t bytes[128];
        };
    }

    void init(const usb::serial::stream *stream);
    void update();
}
