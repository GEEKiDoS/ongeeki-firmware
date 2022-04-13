#pragma once

namespace usb {
    namespace serial {
        class stream {
        public:
            explicit stream(int itf);

            uint8_t read() const;
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
