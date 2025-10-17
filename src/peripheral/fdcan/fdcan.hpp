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
  FDCAN1 = 1,
  FDCAN2 = 2,
};

class FDCAN {
    int8_t receive_message(FDCANChannel channel, fdcan_message_t msg);
    void send_message(const fdcan_message_t &msg);
 private:
    static MessagesCircularBuffer<fdcan_message_t> messages[2];
};

}  // namespace HAL
