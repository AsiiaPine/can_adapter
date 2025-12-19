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
#include "peripheral/led/led.hpp"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;
using HAL::USB;

bool SLCAN::timestamping[FDCANChannel::NUM_CHANNELS] = {false};

char SLCANCommand_to_char(SLCANCommand cmd) {
    return static_cast<char>(cmd);
}

int8_t SLCAN::change_bitrate(uint8_t channel, char char_bitrate) {
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
    FDCAN::set_bitrate(channel, bitrate);
    return 0;
}

int8_t SLCAN::change_custom_bitrate(uint8_t channel, uint8_t time_quantum, uint8_t jump_width,
                                    uint8_t time_segment1, uint8_t time_segment2) {
    return FDCAN::set_custom_bitrate(channel, time_quantum, jump_width, time_segment1, time_segment2);
}

int8_t SLCAN::process_cmd_from_usb(uint8_t channel) {
    uint8_t data[40] = {};
    if (USB::get_message(data,
                            sizeof(data), ENDChar::CHAR_SUCCESS, channel) <= 0)
        return -1;
    uint8_t uint8_buf[1] = {0};
    int8_t ret = 0;
    switch (SLCANCommand(data[0])) {
    case SLCANCommand::TRANSMIT_STANDART: {
        // Transmit standart frame
        slcan_frame_t frame = {.isExtended = false, .isRemote = false};
        ret = transmit_can_frame(frame, data, channel);
        break;
    }
    case SLCANCommand::TRANSMIT_EXTENDED_ALT: {
        // Transmit extended frame
        slcan_frame_t frame = {.isExtended = true, .isRemote = false};
        ret = transmit_can_frame(frame, data, channel);
        break;
    }
    case SLCANCommand::TRANSMIT_EXTENDED: {
        // Transmit extended frame
        slcan_frame_t frame = {.isExtended = true, .isRemote = false};
        ret = transmit_can_frame(frame, data, channel);
        break;
    }
    case SLCANCommand::TRANSMIT_STANDART_RTR: {
        // Transmit standart RTR frame
        slcan_frame_t frame = {.isExtended = false, .isRemote = true};
        ret = transmit_can_frame(frame, data, channel);
        break;
    }
    case SLCANCommand::TRANSMIT_EXTENDED_RTR: {
        // Transmit extended RTR frame
        slcan_frame_t frame = {.isExtended = true, .isRemote = true};
        ret = transmit_can_frame(frame, data, channel);
        break;
    }
    case SLCANCommand::OPEN_CHANNEL: {
        // Open CAN channels
        ret = FDCAN::start(HAL::FDCANChannel(channel));
        break;
    }
    case SLCANCommand::CLOSE_CHANNEL: {
        // Close CAN channels
        ret = FDCAN::stop(HAL::FDCANChannel(channel));
        break;
    }
    case SLCANCommand::SETUP_BITRATE_CMD: {
        ret = change_bitrate(channel, data[1]);
        break;
    }
    case SLCANCommand::SETUP_CUSTOM_BITRATE_CMD: {
        uint8_t settings[4] = {};
        settings[0] = data[1];
        settings[1] = data[2];
        settings[2] = data[3];
        settings[3] = data[4];
        ret = change_custom_bitrate(channel, settings[0], settings[1], settings[2], settings[3]);
        break;
    }
    case SLCANCommand::GET_STATUS: {
        // Get status
        // snprintf(buf, sizeof(buf), "F%02x\r", FDCAN::status);
        FDCAN::check_can_bus(channel);
        // HAL::USB::send_message(buf, strlen(buf), channel);
        return 0;
    }
    case SLCANCommand::ACCEPTANCE_CODE: {
        // Set acceptance code
        // TODO(Asiiapine): Implement
        ret = 0;
        break;
    }
    case SLCANCommand::ACCEPTANCE_MASK: {
        // Set acceptance mask
        // TODO(Asiiapine): Implement
        ret = 0;
        break;
    }
    case SLCANCommand::GET_VERSION: {
        // Get version
        // TODO(Asiiapine): Implement
        ret = 0;
        break;
    }
    case SLCANCommand::SERIAL_NUMBER: {
        // Get serial number
        // TODO(Asiiapine): Implement
        ret = 0;
        break;
    }
    case SLCANCommand::SET_TIMESTAMP_CMD: {
        // Set timestamp
        ret = 0;
        if (data[1] == '0') {
            timestamping[channel] = false;
            break;
        }
        timestamping[channel] = true;
        break;
    }
    default: {
        // Unknown command
        uint8_buf[0] = CHAR_FAIL;
        USB::send_message(uint8_buf, 1, channel);
        return -1;
    }
    }
    if (ret != 0) {
        uint8_buf[0] = CHAR_FAIL;
    } else {
        uint8_buf[0] = CHAR_SUCCESS;
    }
    USB::send_message(uint8_buf, 1, channel);
    return 0;
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
    snprintf(id_hex, sizeof(id_hex), "%X", (unsigned int)msg.id);

    // Format DLC
    char dlc_str[3] = {};
    snprintf(dlc_str, sizeof(dlc_str), "%X", static_cast<int>(msg.dlc));

    // Format data bytes
    for (int i = 0; i < msg.dlc; i++) {
        snprintf(data_hex + 2 * i, 3, "%02X", msg.data[i]);
    }

    char timestamp_str[5] = {0};
    if (timestamping[msg.channel])
        // Format timestamp
        snprintf(timestamp_str, sizeof(timestamp_str), "%04X", static_cast<int>(msg.timestamp));

    // Format the complete message
    int len = snprintf(data, sizeof(data), "%c%s%s%s%s\r",
                        start_char, id_hex, dlc_str, data_hex, timestamp_str);

    if (len < 0 || static_cast<size_t>(len) >= sizeof(data)) {
        return -1;  // Formatting error
    }

    return HAL::USB::send_message(reinterpret_cast<uint8_t*>(data), len, msg.channel);
}

int8_t SLCAN::transmit_can_frame(slcan_frame_t frame, uint8_t* data, uint8_t channel) {
    if (data == nullptr) {
        return -1;
    }

    fdcan_message_t msg = { .channel = channel,
                            .isExtended = frame.isExtended,
                            .isRemote = frame.isRemote};

    uint8_t id_len = frame.isExtended ? 8 : 3;
    for (uint8_t i = 0; i < id_len; i++) {
        msg.id <<= 4;
        msg.id |= charToUint8_t(data[i + 1]);
    }

    msg.dlc = charToUint8_t(data[id_len + 1]);

    for (uint8_t i = 0; i < msg.dlc * 2; i++) {
        if (i % 2 == 0) {
            msg.data[i/2] = charToUint8_t(data[id_len + 2 + i]) << 4;
        } else {
            msg.data[i/2] |= charToUint8_t(data[id_len + 2 + i]);
        }
    }

    HAL::FDCAN::send_message(&msg);
    return 0;
}

void SLCAN::spin() {
    // Process USB commands first
    process_cmd_from_usb(USB::Channels::USB_0);
    process_cmd_from_usb(USB::Channels::USB_1);

    fdcan_message_t test_msg = {0};  // Initialize to prevent garbage
    if (FDCAN::status[0] || FDCAN::status[1]) {
        HAL::LED::set_color(HAL::LEDColor::LED_RED);
    }
    // Check channel 1
    if (HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_1, test_msg) == 0) {
        // Validate message before sending
        if (test_msg.id != 0 && test_msg.dlc <= 8) {
            send_can_to_usb(test_msg);
        }
    }

    // Check channel 2
    if (HAL::FDCAN::receive_message(HAL::FDCANChannel::CHANNEL_2, test_msg) == 0) {
        // Validate message before sending
        if (test_msg.id != 0 && test_msg.dlc <= 8) {
            send_can_to_usb(test_msg);
        }
    }
}
