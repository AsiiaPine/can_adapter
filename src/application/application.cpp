/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "application.hpp"
#include "peripheral/usb/usb.hpp"
#include "peripheral/iwdg/iwdg.hpp"
#include "drivers/slcan/slcan.hpp"
#include "peripheral/led/led.hpp"
#include "main.h"

uint8_t init_msg[] = "USB-CAN Adapter Ready\r\n";
uint8_t test_msg[16] = {};
HAL::fdcan_message_t test_msg2;


__attribute__((noreturn)) void application_entry_point() {
    // uint32_t last_time = HAL_GetTick();
    HAL::LED::init();
    HAL::WatchDog::init();
    uint8_t test_msg[16] = {};
    while (true) {
        // HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_1, test_msg2);
        // HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_2, test_msg2);
        SLCAN::spin();
        // Feed watchdog
        HAL::WatchDog::refresh();
    }
    // init_board_periphery();
    // ModuleManager::init();

// while (true) {
//     ModuleManager::process();
//     blink_board_led();
//     HAL::Watchdog::refresh();
// }
}
