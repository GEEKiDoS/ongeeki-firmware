#pragma once

namespace led_board {
    void init(const usb::serial::stream *stream);
    void update();
}
