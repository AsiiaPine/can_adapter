/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include <cstdio>
#include "main.h"
#include "slcan.hpp"
#include "peripheral/usb/usb.hpp"
#include "peripheral/fdcan/fdcan.hpp"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;
using HAL::USB;

char SLCANCommand_to_char(SLCANCommand cmd) {
    return static_cast<char>(cmd);
}
int8_t SLCAN::change_bitrate(char char_bitrate) {
    uint32_t bitrate = 0;
    switch (char_bitrate) {
    case SLCANBitrate::BITRATE_1M:
        bitrate = 1000000;
        break;
    case SLCANBitrate::BITRATE_800K:
        bitrate = 800000;
        break;
    case SLCANBitrate::BITRATE_500K:
        bitrate = 500000;
        break;
    case SLCANBitrate::BITRATE_250K:
        bitrate = 250000;
        break;
    case SLCANBitrate::BITRATE_125K:
        bitrate = 125000;
        break;
    case SLCANBitrate::BITRATE_100K:
        bitrate = 100000;
        break;
    case SLCANBitrate::BITRATE_50K:
        bitrate = 50000;
        break;
    case SLCANBitrate::BITRATE_20K:
        bitrate = 20000;
        break;
    case SLCANBitrate::BITRATE_10K:
        bitrate = 10000;
        break;
    default:
        return -1;
    }
    FDCAN::set_bitrate(bitrate);
    return 0;
}

int8_t SLCAN::change_custom_bitrate(uint8_t time_quantum, uint8_t jump_width,
                                    uint8_t time_segment1, uint8_t time_segment2) {
    // FDCAN::set_custom_bitrate(time_quantum, jump_width, time_segment1, time_segment2);
    (void)(time_quantum);
    (void)(jump_width);
    (void)(time_segment1);
    (void)(time_segment2);
    return 0;
}

int8_t SLCAN::process_cmd_from_usb() {
    char data[40] = {};
    if (USB::get_message(reinterpret_cast<uint8_t*>(data),
                            sizeof(data), ENDChar::CHAR_SUCCESS) <= 0)
        return -1;
    char buf[6];

    switch (SLCANCommand(data[0])) {
    case SLCANCommand::SETUP_BITRATE_CMD: {
        return change_bitrate(data[1]);
        break;
    }
    case SLCANCommand::SETUP_CUSTOM_BITRATE_CMD: {
        uint8_t settings[4] = {};
        settings[0] = data[1];
        settings[1] = data[2];
        settings[2] = data[3];
        settings[3] = data[4];
        snprintf(buf, sizeof(buf), "set\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return change_custom_bitrate(settings[0], settings[1], settings[2], settings[3]);
        break;
    }

    case SLCANCommand::OPEN_CHANNEL: {
        // Open CAN channels
        FDCAN::start(HAL::FDCANChannel::CHANNEL_1);
        FDCAN::start(HAL::FDCANChannel::CHANNEL_2);
        snprintf(buf, sizeof(buf), "open\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::CLOSE_CHANNEL: {
        // Close CAN channels
        FDCAN::stop(FDCANChannel::CHANNEL_1);
        FDCAN::stop(FDCANChannel::CHANNEL_2);
        snprintf(buf, sizeof(buf), "close\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::TRANSMIT_STANDART: {
        // Transmit standart frame
        slcan_frame_t frame;
        memcpy(&frame, data, sizeof(slcan_frame_t));
        snprintf(buf, sizeof(buf), "ts\r");
        return transmit_can_frame(frame);
        USB::send_message((uint8_t*)buf, strlen(buf));
        break;
    }
    case SLCANCommand::TRANSMIT_EXTENDED: {
        // Transmit extended frame
        slcan_frame_t frame;
        memcpy(&frame, data, sizeof(slcan_frame_t));
        snprintf(buf, sizeof(buf), "Te\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return transmit_can_frame(frame);
        break;
    }
    case SLCANCommand::TRANSMIT_STANDART_RTR: {
        // Transmit standart RTR frame
        slcan_frame_t frame;
        memcpy(&frame, data, sizeof(slcan_frame_t));
        snprintf(buf, sizeof(buf), "tsr\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return transmit_can_frame(frame);
        break;
    }
    case SLCANCommand::TRANSMIT_EXTENDED_RTR: {
        // Transmit extended RTR frame
        slcan_frame_t frame;
        memcpy(&frame, data, sizeof(slcan_frame_t));
        snprintf(buf, sizeof(buf), "Ter\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return transmit_can_frame(frame);
        break;
    }
    case SLCANCommand::GET_STATUS: {
        // Get status
        snprintf(buf, sizeof(buf), "get\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::ACCEPTANCE_CODE: {
        // Set acceptance code
        snprintf(buf, sizeof(buf), "acc\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::ACCEPTANCE_MASK: {
        // Set acceptance mask
        snprintf(buf, sizeof(buf), "am\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::GET_VERSION: {
        // Get version
        snprintf(buf, sizeof(buf), "ver\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::SERIAL_NUMBER: {
        // Get serial number
        snprintf(buf, sizeof(buf), "ser\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    case SLCANCommand::SET_TIMESTAMP_CMD: {
        // Set timestamp
        snprintf(buf, sizeof(buf), "ts\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return 0;
        break;
    }
    default: {
        // Unknown command
        snprintf(buf, sizeof(buf), "unk\r");
        USB::send_message((uint8_t*)buf, strlen(buf));
        return -1;
    }
    }
    return 1;
}

uint8_t charToUint8_t(char ch) {
    if (ch >= 65)
        return ch - 'A' + 10;
    return ch - '0';
}

int8_t SLCAN::send_can_to_usb(fdcan_message_t msg) {
    SLCANCommand start_char = SLCANCommand::TRANSMIT_STANDART;
    char data[28] = {};  // Increased buffer size for safety
    if (msg.dlc > 8) {
        return -1;
    }

    char id_hex[9];
    char data_hex[17] = {0};  // 8 bytes * 2 chars + null terminator

    if (msg.isExtended == 1) {
        start_char = SLCANCommand::TRANSMIT_EXTENDED;
        if (msg.isRemote == true) {
            start_char = SLCANCommand::TRANSMIT_EXTENDED_RTR;
        }
    } else {
        start_char = SLCANCommand::TRANSMIT_STANDART;
        if (msg.isRemote == true) {
            start_char = SLCANCommand::TRANSMIT_STANDART_RTR;
        }
    }

    // Format ID without newline
    snprintf(id_hex, sizeof(id_hex), "%X", msg.id);

    // Format DLC
    char dlc_str[3] = {};
    snprintf(dlc_str, sizeof(dlc_str), "%X", static_cast<int>(msg.dlc));

    // Format data bytes
    for (int i = 0; i < msg.dlc; i++) {
        snprintf(data_hex + 2 * i, 3, "%02X", msg.data[i]);
    }

    // Format the complete message
    int len = snprintf(data, sizeof(data), "%c%s%s%s\r",
                        start_char, id_hex, dlc_str, data_hex);

    if (len < 0 || static_cast<size_t>(len) >= sizeof(data)) {
        return -1;  // Formatting error
    }

    return HAL::USB::send_message(reinterpret_cast<uint8_t*>(data), len);
}

int8_t SLCAN::transmit_can_frame(slcan_frame_t frame) {
    fdcan_message_t msg;
    msg.isExtended = frame.id[0] == 0x1;
    msg.isRemote = frame.command == SLCANCommand::TRANSMIT_EXTENDED_RTR ||
                   frame.command == SLCANCommand::TRANSMIT_STANDART_RTR;
    msg.id = 0;
    for (uint8_t i = 0; i < 8; i++) {
        msg.id |= (frame.id[i] << (4 * (8 - 1 - i)));
    }
    msg.dlc = frame.dlc;
    memcpy(msg.data, frame.data, frame.dlc);
    HAL::FDCAN::send_message(&msg);
    // return send_can_to_usb(msg);
}

void SLCAN::spin() {
    // Process USB commands first
    process_cmd_from_usb();

    // static uint32_t last_time = HAL_GetTick();
    // char buf[] = "SLCAN Idle\r\n";

    // fdcan_message_t test_msg = {0};  // Initialize to prevent garbage

    // if (HAL_GetTick() - last_time > 1000) {
    //     // Check channel 1
    //     if (HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_1, test_msg) == 0) {
    //         // Validate message before sending
    //         if (test_msg.id != 0 && test_msg.dlc <= 8) {
    //             send_can_to_usb(test_msg);
    //         }
    //         last_time = HAL_GetTick();
    //         return;
    //     }

    //     // Check channel 2
    //     if (HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_2, test_msg) == 0) {
    //         // Validate message before sending
    //         if (test_msg.id != 0 && test_msg.dlc <= 8) {
    //             send_can_to_usb(test_msg);
    //         }
    //         last_time = HAL_GetTick();
    //         return;
    //     }
    //     // Send idle message only if no valid messages were processed
    //     HAL::USB::send_message(reinterpret_cast<uint8_t*>(buf), 12);
    //     last_time = HAL_GetTick();
    // }
}
