/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "usbd_cdc_if.h"
#include "main.h"
#include "peripheral/usb/usb.hpp"
#include "common/curcular_buffer.hpp"

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USB RX/TX buffers and indices */
#define MAX_MESSAGES 100
uint8_t HAL::USB::messages_buffer[MAX_MESSAGES] = {};

MessagesCircularBuffer<uint8_t> HAL::USB::messages =
                                    MessagesCircularBuffer<uint8_t>(MAX_MESSAGES, messages_buffer);

/* HAL USBD_CDC functions */
int8_t process_usb_command(uint8_t *data, uint16_t len) {
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, data);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  HAL::USB::messages.push_messages(data, len);

  return USBD_OK;
}

int8_t transmit_complete_callback(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  return USBD_OK;
}
