/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include <cstring>
#include <cstdint>
#include "main.h"


template <typename T> class MessagesCircularBuffer {
 public:
    explicit MessagesCircularBuffer(uint8_t maximum_size, T *messages_buffer) :
                                            max_size(maximum_size), messages(messages_buffer) {
        // Initialize the messages array to zero
        head_idx = 0;
        size = 0;
    }

    inline void push_messages(const T* data, uint8_t number_of_messages) {
        if (head_idx + number_of_messages > max_size) {
            // Wrap around
            uint8_t first_part = max_size - head_idx;
            memcpy(messages + head_idx, data, first_part);
            memcpy(messages, data + first_part, number_of_messages - first_part);
        } else {
            memcpy(messages + head_idx, data, number_of_messages * sizeof(T));
        }
        head_idx = (head_idx + number_of_messages) % max_size;
        size += number_of_messages;
        size = size > max_size ? max_size : size;
    }

    inline void push_message(const T& message) {
        // Direct assignment instead of memcpy for better performance and safety
        enterCriticalSection();

        messages[head_idx] = message;
        head_idx = (head_idx + 1) % max_size;
        if (size < max_size)
            size++;
        exitCriticalSection();
    }

    inline int8_t pop_message(T* message) {
        if (size == 0) {
            return -1;
        }

        uint8_t tail_idx = 0;
        if (head_idx >= size) {
            //  |   | tail  | X | X | X | X | head  |   |
            tail_idx = head_idx - size;
        } else {
            //  | X | head  |   |   |   |   | tail  | X |
            tail_idx = max_size - size + head_idx;
        }

        enterCriticalSection();
        *message = messages[tail_idx];
        size--;
        exitCriticalSection();
        return 0;
    }

    // Number of messages in the buffer
    uint8_t size = 0;

 private:
    uint8_t max_size;
    T *messages;
    // per-instance property to track IRQ disable depth
    static uint32_t irq_disable_depth;

    // Index of the next write position
    uint8_t head_idx = 0;

    inline void enterCriticalSection() {
        __disable_irq();
        irq_disable_depth++;
    }

    inline void exitCriticalSection() {
        if (irq_disable_depth > 0) {
            irq_disable_depth--;
            if (irq_disable_depth == 0)
                __enable_irq();
        }
    }
};

template <typename T> uint32_t MessagesCircularBuffer<T>::irq_disable_depth = 0;
