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
#include "usbd_cdc_if.h"
uint8_t init_msg[] = "USB-CAN Adapter Ready\n";
uint8_t test_msg[16] = {};
HAL::fdcan_message_t test_msg2;


__attribute__((noreturn)) void application_entry_point() {
    // uint32_t last_time = HAL_GetTick();
    HAL::LED::init();
    HAL::WatchDog::init();
    while (true) {
        // if (HAL_GetTick() - last_time > 1000) {
        //     // CDC_Transmit_FS_EndPoint((uint8_t*)"A\n", 2, 0);
        //     // CDC_Transmit_FS_EndPoint((uint8_t*)"B\n", 2, 1);
        //     // CDC_Send(0, (uint8_t*)"A\n", 2);
        //     // CDC_Send(1, (uint8_t*)"B\n", 2);
        //     last_time = HAL_GetTick();
        // }

        SLCAN::spin();

        CDC_TxScheduler();
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
