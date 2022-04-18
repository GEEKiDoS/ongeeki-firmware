#include "stdinclude.hpp"
#include "tusb.h"
#include "serial.hpp"

namespace usb {
    namespace serial {
        stream streams[CFG_TUD_CDC] = {
            stream(0),
#ifdef LED_BOARD_ENABLE
            stream(1),
#endif
        };

        void init() {
            aime_reader::init(&streams[0]);
#ifdef LED_BOARD_ENABLE
            led_board::init(&streams[1]);
#endif
        }

        void update() {
            aime_reader::update();
#ifdef LED_BOARD_ENABLE
            led_board::update();
#endif
        }

        stream::stream(int itf) {
            m_itf = itf;

            printf("stream::ctor(%d)\n", itf);
        }

        void stream::write(uint8_t byte) const {
            if(byte == 0xE0 || byte == 0xD0) {
                tud_cdc_n_write_char(m_itf, (char)0xD0);
                tud_cdc_n_write_char(m_itf, (char)(byte - 1));
            } else {
                tud_cdc_n_write_char(m_itf, (char)byte);
            }
        }

        void stream::write_head() const {
            tud_cdc_n_write_char(m_itf, (char)0xE0);
        }

        bool stream::read(uint8_t &out) const {
            auto byte = (uint8_t) tud_cdc_n_read_char(m_itf);

            if(byte == 0xD0) {
                out = (uint8_t)(tud_cdc_n_read_char(m_itf) + 1);
                return true;
            }

            out = byte;
            return false;
        }

        uint8_t stream::read() const {
            return (uint8_t) tud_cdc_n_read_char(m_itf);
        }

        bool stream::available() const {
            auto avail = tud_cdc_n_available(m_itf);
            if(avail == 1) {
                uint8_t peek;
                assert(tud_cdc_n_peek(m_itf, &peek));

                if(peek == 0xD0)
                    return false;

                return true;
            } else if(avail > 0) {
                return true;
            }

            return false;
        }

        void stream::flush() const {
            tud_cdc_n_write_flush(m_itf);
        }
    }
}
