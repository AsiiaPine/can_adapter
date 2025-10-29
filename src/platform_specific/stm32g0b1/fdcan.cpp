/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdio>
#include "peripheral/fdcan/fdcan.hpp"
#include "fdcan.h"
#include "usbd_cdc_if.h"
#include <stm32g0xx_hal_fdcan.h>
#include "main.h"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
#define MAX_MESSAGES 10


uint8_t get_message_length(FDCAN_RxHeaderTypeDef *rx_header);

fdcan_message_t FDCAN::buffer[2][MAX_MESSAGES] = {{}};

MessagesCircularBuffer<fdcan_message_t> FDCAN::messages[2] = {
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[0], TIM16_FDCAN_IT0_IRQn),
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[1], TIM16_FDCAN_IT0_IRQn)
};

void FDCAN::set_bitrate(uint32_t bitrate) {
    UNUSED(bitrate);
}

void FDCAN::set_custom_bitrate(uint8_t time_quantum, uint8_t jump_width,
                                    uint8_t time_segment1, uint8_t time_segment2) {
    // FDCAN::set_custom_bitrate(time_quantum, jump_width, time_segment1, time_segment2);

    (void)(time_quantum);
    (void)(jump_width);
    (void)(time_segment1);
    (void)(time_segment2);
}

void FDCAN::stop(FDCANChannel channel) {
    if (channel == FDCANChannel::CHANNEL_1) {
        HAL_FDCAN_Stop(&hfdcan1);
    } else if (channel == FDCANChannel::CHANNEL_2) {
        HAL_FDCAN_Stop(&hfdcan2);
    }
}

void FDCAN::start(FDCANChannel channel) {
    if (channel == FDCANChannel::CHANNEL_1) {
        HAL_FDCAN_Start(&hfdcan1);
    } else if (channel == FDCANChannel::CHANNEL_2) {
        HAL_FDCAN_Start(&hfdcan2);
    }
}

int8_t FDCAN::receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t& msg) {
    if (channel > FDCANChannel::CHANNEL_2) {
        return -1;
    }
    if (FDCAN::messages[static_cast<int>(channel) - 1].pop_message(&msg) < 0) {
        return -1;
    }
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

/* CAN Interrupt Callbacks */
// Push received message to circular buffer
void push_can_message(uint8_t channel, FDCAN_RxHeaderTypeDef *rx_header, uint8_t *data) {
    fdcan_message_t msg = {0};  // Initialize all fields to zero

    msg.isExtended = (rx_header->IdType != FDCAN_STANDARD_ID);
    msg.isRemote   = (rx_header->RxFrameType != FDCAN_DATA_FRAME);

    if (msg.isExtended) {
        HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
        msg.id = rx_header->Identifier & 0x1FFFFFFF;
    } else {
        msg.id = rx_header->Identifier & 0x7FF;
    }

    // Skip messages with invalid ID
    if (msg.id == 0) {
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        return;
    }

    if (msg.isRemote)
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);

    msg.dlc = get_message_length(rx_header);
    msg.channel = channel;

    // Validate DLC
    if (msg.dlc > 8) {
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        return;  // Skip invalid messages
    }

    // Copy data safely with bounds checking
    for (uint8_t i = 0; i < msg.dlc; i++) {
        msg.data[i] = data[i];
    }

    // Clear remaining bytes
    for (uint8_t i = msg.dlc; i < 8; i++) {
        msg.data[i] = 0;
    }

    // Only push valid messages
    HAL::FDCAN::messages[channel - 1].push_message(msg);
}

static void check_can_bus(FDCAN_HandleTypeDef *hfdcan) {
    FDCAN_ProtocolStatusTypeDef protocolStatus = {};

    HAL_FDCAN_GetProtocolStatus(hfdcan, &protocolStatus);
    if (protocolStatus.BusOff) {
        CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);
    }
}

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
    push_can_message(channel, &rx_header, rx_data);
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
    push_can_message(channel, &rx_header, rx_data);
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

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    if ((ErrorStatusITs & FDCAN_IT_BUS_OFF) != RESET) {
        check_can_bus(hfdcan);
    }
}

// Helper function to send message via USB
// Function is used to get message length from FDCAN_RxHeaderTypeDef
uint8_t get_message_length(FDCAN_RxHeaderTypeDef *rx_header) {
    if (rx_header == nullptr) {
        return 255;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_0) {
        return 0;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_1) {
        return 1;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_2) {
        return 2;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_3) {
        return 3;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_4) {
        return 4;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_5) {
        return 5;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_6) {
        return 6;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_7) {
        return 7;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_8) {
        return 8;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_12) {
        return 12;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_16) {
        return 16;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_20) {
        return 20;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_24) {
        return 24;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_32) {
        return 32;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_48) {
        return 48;
    }
    if (rx_header->DataLength & FDCAN_DLC_BYTES_64) {
        return 64;
    }
    return 255;  // Invalid length
}
