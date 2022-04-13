#pragma once

namespace aime_reader {
    void init();
    void update(const usb::serial::stream &stream);
}
