# ONGEEKi Firmware
## an Emulated IO4 Firmware for ESP32S2
### IDF Version: 4.4

## Compile and flash
Under IDF 4.4 Environment (run export script first)
```bash
git clone --recurse-submodules https://github.com/GEEKiDoS/ongeeki-firmware.git
cd ongeeki-firmware
idf.py flash
```

## Hardware Pin Definitions
- Main IO: See https://github.com/GEEKiDoS/ongeeki-firmware/blob/master/main/src/hardware.cpp#L10
- Lever: https://github.com/GEEKiDoS/ongeeki-firmware/blob/master/main/src/hardware.cpp#L35
- LED (WS2812): https://github.com/GEEKiDoS/ongeeki-firmware/blob/master/main/src/hardware.cpp#L53
   - Order is left to right
- Card Reader (PN532/I2C mode): https://github.com/GEEKiDoS/ongeeki-firmware/blob/master/main/src/boards/aime_cmd.hpp#L8
- USB: D+ GPIO20, D- GPIO19
