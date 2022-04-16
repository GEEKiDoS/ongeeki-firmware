#include "stdinclude.hpp"
#include "tusb.h"
#include <driver/gpio.h>

namespace usb {
    namespace hid {
        static io4::output_t output_data;

        TaskHandle_t h_add_coin;
        [[noreturn]] static void add_coin_task(void *arg) {
            gpio_set_direction(GPIO_NUM_10, GPIO_MODE_INPUT);
            gpio_set_pull_mode(GPIO_NUM_10, GPIO_PULLUP_ONLY);

            output_data.coin[0].count = 0;

            while (true) {
                if(gpio_get_level(GPIO_NUM_10) == 0) {
                    output_data.coin[0].count ++;
                    printf("coin_task: inserted coin\n");
                }

                vTaskDelay(1000 / portTICK_RATE_MS);
            }
        }

        void init() {
            xTaskCreate(add_coin_task, "test_coin", 4096, nullptr, 5, &h_add_coin);

            memset(&output_data, 0, sizeof(io4::output_t));
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
                    output_data.coin[0].count = 0;
                    output_data.coin[0].condition = io4::coin_condition_t::normal;
                    output_data.coin[1].count = 0;
                    output_data.coin[1].condition = io4::coin_condition_t::normal;
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
