#include "stdinclude.hpp"
#include "tusb.h"

namespace usb {
    namespace hid {
        static io4::output_t output_data;

        void init() {
            output_data.system_status = 0x02;
        }

        void update() {
            if (tud_hid_ready()) {
                tud_hid_report(0x01, &output_data, sizeof(output_data));
            }
        }

        void process_data(const io4::input_t *data) {
            switch (data->cmd) {
                case io4::SET_COMM_TIMEOUT:
                    printf("IO4: SET COMMUNICATE TIMEOUT\n");
                    output_data.system_status = 0x30;
                    break;
                case io4::SET_SAMPLING_COUNT:
                    printf("IO4: SET SAMPLING COUNT\n");
                    output_data.system_status = 0x30;
                    break;
                case io4::CLEAR_BOARD_STATUS:
                    printf("IO4: CLEAR BOARD STATUS\n");
                    output_data.system_status = 0x00;
                    break;
                case io4::SET_GENERAL_OUTPUT:
                    printf("IO4: SET GENERAL OUTPUT\n");

                    break;
                default:
                    break;
            }
        }
    }
}
