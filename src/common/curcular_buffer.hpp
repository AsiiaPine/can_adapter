/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once
#include <cstring>
#include <cstdint>
#include "usbd_cdc_if.h"
#include <vector>

template <typename T> class MessagesCircularBuffer {
 public:
    explicit MessagesCircularBuffer(uint8_t maximum_size) : max_size(maximum_size) {
        // Initialize the messages array to zero
        head_idx = 0;
        size = 0;
        messages.resize(max_size);
    }

    inline void push_message(const T& message) {
        // Direct assignment instead of memcpy for better performance and safety
        messages[head_idx] = message;
        head_idx = (head_idx + 1) % max_size;
        size = (size + 1) % max_size;
    }

    inline int8_t pop_last_message(T* message) {
        if (size == 0) {
            return -1;
        }
        if (size > max_size) {
            size = 0;
            head_idx = 0;
            static char buf[] = {"Buffer underflow or overflow\r\n"};
            CDC_Transmit_FS(reinterpret_cast<uint8_t*>(buf), 28);
            return -2;
        }

        uint8_t tail_idx = 0;
        if (head_idx < size) {
            //  | X | head  |   |   |   |   | tail  | X |
            tail_idx = max_size - size + head_idx;
        } else {
            //  |   | tail  | X | X | X | X | head  |   |
            tail_idx = head_idx - size + 1;
        }

        *message = messages[tail_idx];
        size--;
        return 0;
    }

    // Number of messages in the buffer
    uint8_t size = 0;

 private:
    uint8_t max_size;
    std::vector<T> messages;
    // Index of the next write position
    uint8_t head_idx = 0;
};
