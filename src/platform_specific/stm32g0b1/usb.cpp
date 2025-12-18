/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdint>
#include <cstdio>
#include "common/algorithms.h"
#include "usbd_cdc_if.h"
#include "main.h"
#include "peripheral/usb/usb.hpp"

using HAL::USB;
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USB RX/TX buffers and indices */
#define MAX_MESSAGES 100
uint8_t HAL::USB::messages_buffer[2][MAX_MESSAGES] = {};

MessagesCircularBuffer<uint8_t> HAL::USB::messages[2] = {
    MessagesCircularBuffer<uint8_t>(MAX_MESSAGES, messages_buffer[0], USB_UCPD1_2_IRQn),
    MessagesCircularBuffer<uint8_t>(MAX_MESSAGES, messages_buffer[1], USB_UCPD1_2_IRQn)
};

int8_t USB::get_message(uint8_t *data, uint16_t len, char last_char, uint8_t channel) {
    if (channel >= Channels::NUM_CHANNELS) {
        return -1;
    }
    if (messages[channel].size == 0) {
        return -1;
    }
    uint16_t bytes_read = 0;
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte;
        if (messages[channel].pop_message(&byte) != 0) {
            // Error reading from buffer
            return -1;
        }

        data[i] = byte;
        bytes_read++;

        if (byte == last_char) {
            return bytes_read;  // Successfully read complete message
        }
        if (byte == '\a') {
            return -2;  // Error character received
        }
    }

    // If we get here, we've read len bytes but didn't find the terminator
    return bytes_read > 0 ? 0 : -1;
}

int8_t USB::send_message(uint8_t *data, uint16_t len, uint8_t channel) {
    if (channel >= Channels::NUM_CHANNELS) {
        return -1;
    }
    CDC_Send(channel, data, len);
    return HAL_OK;
}
