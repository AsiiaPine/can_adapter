/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#pragma once
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include "App/usbd_cdc_if.h"
#include "Inc/main.h"
#include "peripheral/usb/usb.hpp"

using HAL::USB;
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t usb_rx_buffer[USB_BUFFER_SIZE];
uint8_t usb_tx_buffer[USB_BUFFER_SIZE];
uint8_t usb_rx_index = 0;
uint8_t usb_tx_index = 0;

int8_t USB::get_message(uint8_t *data, uint16_t len, char last_char) {
    if (usb_rx_index > 0) {
        uint16_t data_to_copy = min(len, usb_rx_index);
        memccpy(data, usb_rx_buffer, '\r', data_to_copy);
        usb_rx_index -= data_to_copy;
        return 0;
    }
    return -1;
}

int8_t USB::send_message(uint8_t *data, uint16_t len) {
    if (CDC_Transmit_FS(data, len) == USBD_OK) {
        return 0;
    }
    return -1;
}

/* HAL USBD_CDC functions */
int8_t process_usb_command(uint8_t *data, uint16_t len) {
// Accumulate received data for CAN protocol processing
  if (usb_rx_index + *len < USB_BUFFER_SIZE) {
    memcpy(&usb_rx_buffer[usb_rx_index], data, *len);
    usb_rx_index += *len;
  } else {
    // Buffer overflow, reset index
    usb_rx_index = 0;
    memcpy(&usb_rx_buffer[usb_rx_index], data, *len);
    // Optionally, you can handle the overflow case here (e.g., log an error)
  }

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &data[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}

int8_t transmit_complete_callback(uint8_t *Buf, uint32_t *Len, uint8_t epnum) {
  return USBD_OK;
}
