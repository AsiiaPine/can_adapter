/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once
#define MAX_MESSAGES 10  // Reduced for STM32G0B1 memory constraints
#include <cstring>
#include <cstdint>

template <typename T> class MessagesCircularBuffer {
 public:
    explicit MessagesCircularBuffer(uint8_t maximum_size) : max_size(maximum_size) {
        // Initialize the messages array to zero
        next_id = 0;
        size = 0;
    }

    inline void push_message(const T& message) {
        // Direct assignment instead of memcpy for better performance and safety
        messages[next_id] = message;
        next_id++;
        size++;
        next_id %= max_size;
        if (size >= max_size) {
            size = max_size;
        }
    }

    inline int8_t pop_last_message(T* message) {
        if (size == 0) {
            return -1;
        }

        *message = messages[last_index];
        size--;
        last_index++;
        last_index %= max_size;
        return 0;
    }

    uint8_t size = 0;

 private:
    uint8_t max_size;
    T messages[MAX_MESSAGES];
    uint8_t next_id = 0;
    uint8_t last_index = 0;
};
