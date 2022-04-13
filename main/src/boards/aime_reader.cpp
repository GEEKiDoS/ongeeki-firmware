#include "stdinclude.hpp"

namespace aime_reader {
    void init() {

    }

    void update(const usb::serial::stream &stream) {
        int c = 0;

        while(stream.available()) {
            stream.write(stream.read());
            c++;
        }

        if(c > 0)
            printf("aime reader recv %d bytes\n", c);

        stream.flush();
    }
}
