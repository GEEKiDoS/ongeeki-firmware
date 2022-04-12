idf_build_get_property(target IDF_TARGET)

set(srcs)
set(includes_public)
set(includes_private)
set(compile_options)


if (target STREQUAL "esp32s3")
    set(tusb_mcu "OPT_MCU_ESP32S3")
    set(tusb_family "esp32sx")
elseif (target STREQUAL "esp32s2")
    set(tusb_mcu "OPT_MCU_ESP32S2")
    set(tusb_family "esp32sx")
else ()
    # CONFIG_TINYUSB dependency has been garanteed by Kconfig logic,
    # So it's not possible that cmake goes here
    message(FATAL_ERROR "TinyUSB is not support on ${target}.")
    return()
endif ()

list(APPEND compile_options
        "-DCFG_TUSB_MCU=${tusb_mcu}"
        "-DCFG_TUSB_DEBUG=${CONFIG_TINYUSB_DEBUG_LEVEL}"
        )

idf_component_get_property(freertos_component_dir freertos COMPONENT_DIR)

list(APPEND includes_private
        "tinyusb/hw/bsp/"
        "tinyusb/src/"
        "tinyusb/src/device"
        )

list(APPEND includes_public
        "tinyusb/src/"
        "include"
        # The FreeRTOS API include convention in tinyusb is different from esp-idf
        "${freertos_component_dir}/include/freertos"
        )

list(APPEND srcs
        "tinyusb/src/portable/espressif/${tusb_family}/dcd_${tusb_family}.c"
        "tinyusb/src/class/cdc/cdc_device.c"
        "tinyusb/src/class/hid/hid_device.c"
        "tinyusb/src/class/midi/midi_device.c"
        "tinyusb/src/class/msc/msc_device.c"
        "tinyusb/src/class/vendor/vendor_device.c"
        "tinyusb/src/common/tusb_fifo.c"
        "tinyusb/src/device/usbd_control.c"
        "tinyusb/src/device/usbd.c"
        "tinyusb/src/tusb.c"
        )

idf_component_register(SRCS ${srcs}
        INCLUDE_DIRS ${includes_public}
        PRIV_INCLUDE_DIRS ${includes_private}
        PRIV_REQUIRES "vfs" "usb"
        )

target_compile_options(${COMPONENT_LIB} PRIVATE ${compile_options})
