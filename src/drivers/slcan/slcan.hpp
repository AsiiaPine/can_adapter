/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once
#ifndef SLCAN_HPP_
#define SLCAN_HPP_
#include "peripheral/fdcan/fdcan.hpp"
#include "peripheral/usb/usb.hpp"

enum ENDChar: char {
  CHAR_SUCCESS = '\r',
  FCHAR_AIL = '\a',
};

enum SLCANCommand: char {
  SETUP_BITRATE_CMD = 'S',
  SETUP_CUSTOM_BITRATE_CMD = 's',
  OPEN_CHANNEL = 'O',
  CLOSE_CHANNEL = 'C',
  TRANSMIT_STANDART = 't',
  TRANSMIT_EXTENDED = 'T',
  TRANSMIT_EXTENDED_ALT = 'x',  // x is an alternative extended message identifier for CANDapter
  TRANSMIT_STANDART_RTR = 'r',
  TRANSMIT_EXTENDED_RTR = 'R',
  TRANSMIT_FD_STANDART = 'd',
  TRANSMIT_FD_EXTENDED = 'D',
  TRANSMIT_FD_STANDART_WITH_BITRATE_SWITCH = 'b',
  TRANSMIT_FD_EXTENDED_WITH_BITRATE_SWITCH = 'B',
  GET_STATUS = 'F',
  ACCEPTANCE_CODE = 'M',
  ACCEPTANCE_MASK = 'm',
  GET_VERSION = 'V',
  SERIAL_NUMBER = 'N',
  SET_TIMESTAMP_CMD = 'Z',
};

char SLCANCommand_to_char(SLCANCommand cmd);

enum SLCANBitrate: char {
  BITRATE_10K = '0',
  BITRATE_20K = '1',
  BITRATE_50K = '2',
  BITRATE_100K = '3',
  BITRATE_125K = '4',
  BITRATE_250K = '5',
  BITRATE_500K = '6',
  BITRATE_800K = '7',
  BITRATE_1M = '8',
};

typedef struct {
  bool isExtended;
  bool isRemote;
} slcan_frame_t;


class SLCAN {
 public:
    static void spin();
 private:
    static int8_t send_can_to_usb(HAL::fdcan_message_t msg);
    static int8_t process_cmd_from_usb(uint8_t channel);
    static int8_t change_bitrate(char bitrate);
    static int8_t change_custom_bitrate(uint8_t time_quantum, uint8_t jump_width,
                                        uint8_t time_segment1, uint8_t time_segment2);
    static int8_t transmit_can_frame(slcan_frame_t frame, uint8_t* data, uint8_t channel);
    static int8_t process_slcan_frame(char *data);
    static bool timestamping;
};

#endif /* SLCAN_HPP_ */
