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
  uint32_t id;
  uint8_t dlc;
  uint8_t data[8];
  uint8_t channel;  // 1 for FDCAN1, 2 for FDCAN2
  bool isExtended;
  bool isRemote;
} fdcan_message_t;

enum class FDCANChannel: uint8_t {
  CHANNEL_1 = 1,
  CHANNEL_2 = 2,
};

class FDCAN {
    int8_t receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t msg);
    void send_message(const HAL::fdcan_message_t &msg);
 public:
    static MessagesCircularBuffer<HAL::fdcan_message_t> messages[2];
};

// Forward declarations for callback functions
};  // namespace HAL
