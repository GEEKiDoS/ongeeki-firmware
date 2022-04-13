#pragma once

#include <cstdio>
#include <functional>
#include <new>

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <esp_err.h>

#include "usb/usb.hpp"
#include "usb/hid.hpp"
#include "usb/serial.hpp"

#include "boards/aime_reader.hpp"
#include "boards/led_board.hpp"
