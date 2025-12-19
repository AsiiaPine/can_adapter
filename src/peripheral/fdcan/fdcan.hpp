/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include <cstdint>
#include "common/curcular_buffer.hpp"

namespace HAL {

enum FDCAN_STATUS_BITS: uint8_t {
  RX_FIFO_FULL   = 0,
  TX_FIFO_FULL   = 1,
  ERROR_WARNING  = 2,
  DATA_OVERRUN   = 3,
  NO_CHANGE      = 4,
  ERROR_PASSIVE  = 5,
  ARBITRATION_LOST = 6,
  BUS_ERROR      = 7,
};

typedef struct {
  uint32_t id = 0;
  uint8_t dlc = 0;
  uint8_t data[8] = {0};
  uint8_t channel = 0;  // 1 for FDCAN1, 2 for FDCAN2
  uint8_t isExtended = 0;
  uint8_t isRemote = 0;
  uint16_t timestamp = 0;
} fdcan_message_t;

enum FDCANChannel: uint8_t {
  CHANNEL_1 = 0,
  CHANNEL_2 = 1,
  NUM_CHANNELS = 2
};

class FDCAN {
 public:
    static uint8_t status;
    static fdcan_message_t buffer[2][10];
    static void stop(FDCANChannel channel);
    static void start(FDCANChannel channel);
    static int8_t receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t& msg);
    static void send_message(HAL::fdcan_message_t *msg);
    static MessagesCircularBuffer<HAL::fdcan_message_t> messages[2];
    static void set_bitrate(uint8_t channel, uint32_t bitrate);
    static void set_custom_bitrate(uint8_t channel, uint8_t time_quantum, uint8_t jump_width,
                                    uint8_t time_segment1, uint8_t time_segment2);
    static void PrintCANStatus(void);
};

};  // namespace HAL
