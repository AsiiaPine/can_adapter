/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdint>
#include <cstdio>
#include "main.h"
#include "peripheral/usb/usb.hpp"
#include <iostream>

using HAL::USB;

int8_t USB::get_message(uint8_t *data, uint16_t len, char last_char) {
    if (messages.size == 0) {
        return -1;
    }
    for (uint16_t i = 0; i < len; i++) {
        if (messages.size == 0) {
            return 0;
        }
        messages.pop_message(data + i);
        if (data[i] == last_char) {
            return 0;
        }
        if (data[i] == '\a') {
            return -1;
        }
    }
    return 0;
}

int8_t USB::send_message(uint8_t *data, uint16_t len) {
    const char *data_str = reinterpret_cast<const char*>(data);
    std::cout << "Sending message: " << std::string(data_str, len) << std::endl;
    return 0;
}
