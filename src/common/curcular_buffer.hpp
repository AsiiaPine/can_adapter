/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once
#define MAX_MESSAGES 100
#include <cstring>
template <typename T> class MessagesCircularBuffer {
 public:
    MessagesCircularBuffer(uint8_t maximum_size) : max_size(maximum_size) {}

    inline void push_message(T message, uint8_t len = sizeof(T)) {
        memcpy(&messages[next_id], &message, len);
        next_id++;
        size++;
        if (next_id >= max_size) {
            next_id = 0;
        }
        if (size >= max_size) {
            size = max_size;
        }
    }

    inline int8_t pop_last_message(T* message) {
        uint8_t id = 0;
        if (size == 0) {
            return -1;
        }
        if (next_id < size) {
            id = max_size - size + next_id;
        } else {
            id = next_id - size + 1;
        }
        memcpy(&messages[id], message, sizeof(T));
        size--;
        return 0;
    }
    uint8_t size = 0;


 private:
    uint8_t max_size;
    T messages[MAX_MESSAGES];
    uint8_t next_id = 0;
};
