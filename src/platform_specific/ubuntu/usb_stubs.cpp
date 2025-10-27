/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "main.h"
#include "peripheral/usb/usb.hpp"
#include "common/curcular_buffer.hpp"

/* USB RX/TX buffers and indices */
#define MAX_MESSAGES 100
uint8_t HAL::USB::messages_buffer[MAX_MESSAGES] = {};
MessagesCircularBuffer<uint8_t> HAL::USB::messages =
                                    MessagesCircularBuffer<uint8_t>(MAX_MESSAGES, messages_buffer);

/* HAL USBD_CDC functions */
int8_t process_usb_command(uint8_t *data, uint16_t len) {
  (void)(data);
  (void)(len);
  return 0;
}

int8_t transmit_complete_callback(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  (void)(Buf);
  (void)(Len);
  (void)(epnum);
  return 0;
}
