#pragma once

namespace led_board {
    void init();
    void update(const usb::serial::stream &stream);
}
