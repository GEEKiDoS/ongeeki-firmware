#include "stdinclude.hpp"

namespace led_board {
    void init() {

    }

    void update(const usb::serial::stream &stream) {
        int c = 0;

        while(stream.available()) {
            stream.write(stream.read());
            c++;
        }

        if(c > 0)
            printf("led board recv %d bytes\n", c);

        stream.flush();
    }
}
