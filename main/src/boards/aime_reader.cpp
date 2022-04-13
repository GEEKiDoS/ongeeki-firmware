#include "stdinclude.hpp"


namespace aime_reader {
    const usb::serial::stream *stream;

    void init(const usb::serial::stream *input) {
        stream = input;
    }

    void on_packet(cAiMeNFCRW::sNfcCommand &req) {
        switch(req.codeCommand) {
            default:
                break;
        }
    }

    cAiMeNFCRW::sNfcCommand req;
    uint8_t len, checksum;

    void update() {

    }
}
