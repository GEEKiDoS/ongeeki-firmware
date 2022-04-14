#include "stdinclude.hpp"
#include "tusb.h"

namespace usb {
    namespace hid {
        static io4::output_t output_data;

        void init() {
            output_data.system_status = 0x02;

            set_coin(16);
        }

        void update() {
            if (tud_hid_ready()) {
                tud_hid_report(0x01, &output_data, sizeof(output_data));
            }
        }

        void set_coin(uint8_t coin_count, size_t slot) {
            if(slot >= 2) return;

            output_data.coin[slot] = uint16_t ((coin_count << 8) | (coin_count >> 14));
        }

        void process_data(const io4::input_t *data) {
            switch (data->cmd) {
                case io4::SET_COMM_TIMEOUT:
                    printf("IO4: Set Communicate Timeout\n");
                    output_data.system_status = 0x30;
                    break;
                case io4::SET_SAMPLING_COUNT:
                    printf("IO4: Set Sampling Count\n");
                    output_data.system_status = 0x30;
                    break;
                case io4::CLEAR_BOARD_STATUS:
                    printf("IO4: Clear Board Status\n");
                    output_data.system_status = 0x00;
                    break;
                case io4::SET_GENERAL_OUTPUT:
                    printf("IO4: Set General Output\n");
                    break;
                default:
                    break;
            }
        }
    }
}
