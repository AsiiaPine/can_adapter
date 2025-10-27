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
    static void spin();
    static int8_t get_message(uint8_t *data, uint16_t len, char last_char = '\r');
    static int8_t send_message(uint8_t *data, uint16_t len);
    static MessagesCircularBuffer<uint8_t> messages;
    static MessagesCircularBuffer<uint8_t> rx_messages;
 private:
    static uint8_t messages_buffer[100];
    static uint8_t rx_messages_buffer[100];
};
}  // namespace HAL
