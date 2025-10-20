/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "application.hpp"
#include <usbd_cdc_if.h>
#include "main.h"
#include "peripheral/usb/usb.hpp"


extern IWDG_HandleTypeDef hiwdg;
uint8_t init_msg[] = "USB-CAN Adapter Ready\r\n";
uint8_t test_msg[16] = {};

__attribute__((noreturn)) void application_entry_point() {
    uint32_t last_time = HAL_GetTick();
    while (true) {
        if (HAL_GetTick() - last_time > 1000) {
            HAL::USB::send_message(init_msg, sizeof(init_msg) - 1);
            HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
            last_time = HAL_GetTick();
            // CDC_Transmit_FS(init_msg, sizeof(init_msg) - 1);
        }

        if (HAL::USB::get_message(test_msg, sizeof(test_msg) - 1) == 0) {
            HAL::USB::send_message(test_msg, sizeof(test_msg) - 1);
            HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        }
        // Feed watchdog
        HAL_IWDG_Refresh(&hiwdg);
    }
    // init_board_periphery();
    // ModuleManager::init();

// while (true) {
//     ModuleManager::process();
//     blink_board_led();
//     HAL::Watchdog::refresh();
// }
}
