/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdint>
#include <cstdio>
#include "common/algorithms.h"
#include "usbd_cdc_if.h"
#include "main.h"
#include "peripheral/usb/usb.hpp"

using HAL::USB;
extern USBD_HandleTypeDef hUsbDeviceFS;

int8_t USB::get_message(uint8_t *data, uint16_t len, char last_char) {
    if (messages.size == 0) {
        return -1;
    }
    for (uint16_t i = 0; i < len; i++) {
        if (messages.size == 0) {
            return 0;
        }
        messages.pop_last_message(data + i);
        if (data[i] == last_char) {
            return 0;
        }
    }
    return 0;
}

int8_t USB::send_message(uint8_t *data, uint16_t len) {
    if (CDC_Transmit_FS(data, len) == USBD_OK) {
        return 0;
    }
    return -1;
}
