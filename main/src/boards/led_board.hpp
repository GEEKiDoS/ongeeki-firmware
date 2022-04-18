#pragma once
#ifdef LED_BOARD_ENABLE
namespace led_board {
    void init(const usb::serial::stream *stream);
    void update();
}
#endif
