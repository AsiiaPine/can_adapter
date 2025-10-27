/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include <iostream>
#include "main.h"
#include "peripheral/fdcan/fdcan.hpp"

using HAL::FDCAN;
using HAL::fdcan_message_t;
void FDCAN::set_bitrate(uint32_t bitrate) {
    (void)(bitrate);
}
#define MAX_MESSAGES 10
fdcan_message_t FDCAN::buffer[2][MAX_MESSAGES] = {{}};

MessagesCircularBuffer<fdcan_message_t> FDCAN::messages[2] = {
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[0]),
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[1])
};

void make_message(HAL::FDCANChannel channel, HAL::fdcan_message_t* msg) {
    msg->channel = static_cast<uint8_t>(channel);
    msg->id = 0x123;
    msg->dlc = 8;
    for (uint8_t i = 0; i < msg->dlc; i++) {
        msg->data[i] = i;
    }
    msg->isExtended = 0;
    msg->isRemote = 0;
}

void push_message(HAL::FDCANChannel channel, HAL::fdcan_message_t& msg) {
    make_message(channel, &msg);
    FDCAN::messages[static_cast<int>(channel) - 1].push_message(msg);
}

int8_t FDCAN::receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t& msg) {
    push_message(channel, msg);
    if (FDCAN::messages[static_cast<int>(channel) - 1].pop_message(&msg) < 0) {
        return -1;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "CAN%d: got a message %X\r\n", static_cast<int>(channel),
                                                                (msg.id));
    std::cout << buf << std::endl;
    return 0;
}

void FDCAN::send_message(fdcan_message_t *msg) {
    std::cout << "Sending message on CAN" << static_cast<int>(msg->channel)
              << " with ID: " << std::hex << msg->id << std::dec << std::endl;
}

void FDCAN::stop(FDCANChannel channel) {
    std::cout << "Stopping CAN" << static_cast<int>(channel) << std::endl;
}

void FDCAN::start(FDCANChannel channel) {
    std::cout << "Starting CAN" << static_cast<int>(channel) << std::endl;
}
