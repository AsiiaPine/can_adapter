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
    uint32_t last_time = HAL_GetTick();
    HAL::LED::init();
    HAL::WatchDog::init();
    while (true) {
        if (HAL_GetTick() - last_time > 1000) {
            last_time = HAL_GetTick();
            HAL::LED::toggle(HAL::LEDColor::LED_BLUE);
        }
        SLCAN::spin();

        CDC_TxScheduler();
        // Feed watchdog
        HAL::WatchDog::refresh();
    }
}
