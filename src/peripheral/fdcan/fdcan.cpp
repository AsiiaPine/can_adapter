/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdio>
#include "fdcan.hpp"
#include "fdcan.h"
#include "usbd_cdc_if.h"
#include <stm32g0xx_hal_fdcan.h>
#include "main.h"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

void FDCAN::set_bitrate(uint32_t bitrate) {
    UNUSED(bitrate);
}

MessagesCircularBuffer<fdcan_message_t> FDCAN::messages[2] = {
    MessagesCircularBuffer<fdcan_message_t>(CAN_MAX_MESSAGES),
    MessagesCircularBuffer<fdcan_message_t>(CAN_MAX_MESSAGES)
};

// Global access to messages for callback functions
static MessagesCircularBuffer<fdcan_message_t>* get_messages() {
    return FDCAN::messages;
}

int8_t FDCAN::receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t msg) {
    if (FDCAN::messages[static_cast<int>(channel) - 1].pop_last_message(&msg) < 0) {
        return -1;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "CAN%d: got a message %d\r\n", static_cast<int>(channel),
                                                                int(msg.id));
    CDC_Transmit_FS(reinterpret_cast<uint8_t*>(buf), strlen(buf));
    return 0;
}

void FDCAN::send_message(fdcan_message_t *msg) {
    FDCAN_HandleTypeDef *hfdcan = (msg->channel == 1) ? &hfdcan1 : &hfdcan2;
    FDCAN_TxHeaderTypeDef tx_header;
    tx_header.Identifier = msg->id;
    tx_header.IdType = msg->isExtended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
    tx_header.TxFrameType = msg->isRemote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
    tx_header.DataLength = msg->dlc << 16;
    // 10  0000   0000  0000  0000
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, msg->data);
}

uint8_t get_message_length(FDCAN_RxHeaderTypeDef rx_header) {
    uint8_t length = rx_header.DataLength << 16;
    if (rx_header.DataLength < GPIO_PIN_9) {
        return length;
    }
    switch (rx_header.DataLength) {
    case FDCAN_DLC_BYTES_12:
        length = 12;
        break;
    case FDCAN_DLC_BYTES_16:
        length = 16;
        break;
    case FDCAN_DLC_BYTES_20:
        length = 20;
        break;
    case FDCAN_DLC_BYTES_24:
        length = 24;
        break;
    case FDCAN_DLC_BYTES_32:
        length = 32;
        break;
    case FDCAN_DLC_BYTES_48:
        length = 48;
        break;
    case FDCAN_DLC_BYTES_64:
        length = 64;
        break;
    default:
        length = 0;
        break;
    }
    return length;
}

/* HAL FD CAN Callbacks */
void push_can_message(uint8_t channel, FDCAN_RxHeaderTypeDef rx_header, uint8_t *data) {
    fdcan_message_t msg = {0};

    msg.isExtended = (rx_header.IdType == FDCAN_EXTENDED_ID);
    msg.isRemote   = (rx_header.RxFrameType == FDCAN_REMOTE_FRAME);

    if (msg.isExtended)
        HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
    if (msg.isRemote)
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);

    msg.id = rx_header.Identifier & (msg.isExtended ? 0x1FFFFFFF : 0x7FF);

    msg.dlc = get_message_length(rx_header);
    msg.channel = channel;
    memcpy(msg.data, data, msg.dlc);
    HAL::FDCAN::messages[channel - 1].push_message(msg);
}

/* CAN Interrupt Callbacks */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    // Debug: Toggle green LED to indicate interrupt was called
    HAL_GPIO_TogglePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin);

    uint8_t channel = 1;
    if (hfdcan == &hfdcan2) {
        channel = 2;
    }
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == RESET) {
        return;
    }
    uint8_t rx_data[64];
    FDCAN_RxHeaderTypeDef rx_header;
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        return;
    }
    push_can_message(channel, rx_header, rx_data);
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs) {
    // Debug: Toggle blue LED to indicate interrupt was called
    HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
    uint8_t channel = 1;
    if (hfdcan == &hfdcan2) {
        channel = 2;
    }
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) == RESET) {
        return;
    }
    uint8_t rx_data[8];
    FDCAN_RxHeaderTypeDef rx_header;
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        return;
    }
    push_can_message(channel, rx_header, rx_data);
}

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t /*TxEventFifoITs*/) {
    // Optional: Handle transmission events
    // UNUSED(hfdcan);
    uint8_t error_msg[19];

    if (hfdcan == &hfdcan1) {
        // Toggle red LED to indicate CAN1 error
        // HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        // Send error message via USB
        memcpy(error_msg, "Retransmit: CAN1\r\n", sizeof(error_msg));
    } else if (hfdcan == &hfdcan2) {
        // Toggle red LED to indicate CAN2 error
        // HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        // Send error message via USB
        memcpy(error_msg, "Retransmit: CAN2\r\n", sizeof(error_msg));
    }
    CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
    // Handle CAN errors
    if (hfdcan == &hfdcan1) {
        // Toggle red LED to indicate CAN1 error
        // HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        // Send error message via USB
        uint8_t error_msg[] = "CAN1 Error\r\n";
        CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
    } else if (hfdcan == &hfdcan2) {
        // Toggle red LED to indicate CAN2 error
        // HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        // Send error message via USB
        uint8_t error_msg[] = "CAN2 Error\r\n";
        CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
    }
    uint8_t error_msg[19];
    snprintf(reinterpret_cast<char*>(error_msg), sizeof(error_msg),
                                "CAN Error %d\r\n", static_cast<int>(hfdcan->ErrorCode));
    CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
}
