#pragma once

namespace hardware {
    struct switch_map_t {
        uint8_t player;
        uint8_t bit;
    };

    void init();
    void update(usb::io4::output_t &data);
    void led_update(const uint8_t *payload);
}
