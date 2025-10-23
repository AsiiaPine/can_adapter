/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include "main.h"
#include <cstdint>
#include "common/curcular_buffer.hpp"

namespace HAL {

typedef struct {
  uint32_t id = 0;
  uint8_t dlc = 0;
  uint8_t data[8] = {0};
  uint8_t channel = 0;  // 1 for FDCAN1, 2 for FDCAN2
  uint8_t isExtended = 0;
  uint8_t isRemote = 0;
} fdcan_message_t;

enum class FDCANChannel: uint8_t {
  CHANNEL_1 = 1,
  CHANNEL_2 = 2,
};

class FDCAN {
 public:
    static int8_t receive_message(HAL::FDCANChannel channel, const HAL::fdcan_message_t& msg);
    static void send_message(HAL::fdcan_message_t *msg);
    static MessagesCircularBuffer<HAL::fdcan_message_t> messages[2];
    static void set_bitrate(uint32_t bitrate);
};

};  // namespace HAL
