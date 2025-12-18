/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include <cstdint>
#include "common/curcular_buffer.hpp"

namespace HAL {

class USB {
 public:
    enum Channels: uint8_t {
      USB_0 = 0,
      USB_1 = 1,
      NUM_CHANNELS = 2
    };
    static void spin();
    static int8_t get_message(uint8_t *data, uint16_t len, char last_char = '\r', uint8_t channel = USB_0);
    static int8_t send_message(uint8_t *data, uint16_t len, uint8_t channel);
    static MessagesCircularBuffer<uint8_t> messages[2];
 private:
    static uint8_t messages_buffer[2][100];
};
}  // namespace HAL
