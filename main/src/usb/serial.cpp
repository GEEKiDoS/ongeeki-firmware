#include "stdinclude.hpp"
#include "tusb.h"
#include "serial.hpp"


namespace usb {
    namespace serial {
        stream readers[CFG_TUD_CDC] = {
                stream(0),
                stream(1),
        };

        void init() {

        }

        void update() {
            led_board::update(readers[0]);
            aime_reader::update(readers[1]);
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

        uint8_t stream::read() const {
            auto byte = (uint8_t) tud_cdc_n_read_char(m_itf);

            if(byte == 0xD0) {
                return (uint8_t)(tud_cdc_n_read_char(m_itf) + 1);
            }

            return byte;
        }

        bool stream::available() const {
            auto avail = tud_cdc_n_available(m_itf);
            if(avail == 1) {
                uint8_t peek;
                if(!tud_cdc_n_peek(m_itf, &peek)) assert(false);

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
