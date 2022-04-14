#pragma once

namespace usb {
    namespace serial {
        class stream {
        public:
            explicit stream(int itf);

            // no escape
            uint8_t read() const;
            // with escape, return value is the value is escaped
            bool read(uint8_t &out) const;
            void write(uint8_t byte) const;
            void write_head() const;
            bool available() const;

            void flush() const;

        private:
            int m_itf;
        };

        typedef void (* update_cb_t)(const stream &stream);

        void init();
        void update();
    }
}
