/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include <cstdio>
#include "STM32G0xx_HAL_Driver/Inc/stm32g0xx_hal_fdcan.h"
#include "peripheral/fdcan/fdcan.hpp"
#include "Inc/fdcan.h"
#include "App/usbd_cdc_if.h"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;

extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan2;


static fdcan_message_t can_rx_queue[CAN_MAX_MESSAGES];
MessagesCircularBuffer<fdcan_message_t> FDCAN::messages[2] = {
    MessagesCircularBuffer<fdcan_message_t>(CAN_MAX_MESSAGES),
    MessagesCircularBuffer<fdcan_message_t>(CAN_MAX_MESSAGES)
};

static uint8_t can_rx_queue_head = 0;
static uint8_t can_rx_queue_tail = 0;

static uint32_t can1_rx_count = 0;
static uint32_t can2_rx_count = 0;
static uint32_t can1_tx_count = 0;
static uint32_t can2_tx_count = 0;

static uint8_t can1_terminator_enabled = 0;
static uint8_t can2_terminator_enabled = 0;


int8_t FDCAN::receive_message(FDCANChannel channel, fdcan_message_t msg) {
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::FDCAN1) ? &hfdcan1 : &hfdcan2;
    FDCAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
        uint8_t rx_msg[64];
        int len = snprintf((char*)rx_msg, sizeof(rx_msg), "CAN%d: got a message\r\n", channel);
        CDC_Transmit_FS(rx_msg, len);
        // Check if we have space in the queue
        uint8_t next_head = (can_rx_queue_head + 1) % CAN_MAX_MESSAGES;
        if (next_head != can_rx_queue_tail) {
            // fdcan_message_t *msg = &can_rx_queue[can_rx_queue_head];
            msg.id = rx_header.Identifier;
            msg.dlc = rx_header.DataLength;
            msg.channel = (uint8_t)channel;
            memcpy(msg.data, rx_data, 8);
            can_rx_queue_head = next_head;
            if (channel == FDCANChannel::FDCAN1) {
                can1_rx_count++;
            } else {
                can2_rx_count++;
            }
            return;
        }
    }
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rx_header, rx_data) == HAL_OK) {
        uint8_t rx_msg[64];
        int len = snprintf(reinterpret_cast<char*>(rx_msg), sizeof(rx_msg),
                                            "CAN%d: got a message\r\n", channel);
        CDC_Transmit_FS(rx_msg, len);

        // Check if we have space in the queue
        uint8_t next_head = (can_rx_queue_head + 1) % CAN_MAX_MESSAGES;
        if (next_head != can_rx_queue_tail) {
            msg.id = rx_header.Identifier;
            msg.dlc = rx_header.DataLength;
            msg.channel = (uint8_t)channel;
            memcpy(msg.data, rx_data, 8);
            can_rx_queue_head = next_head;
            can2_rx_count++;
        }
        return;
    }
    CDC_Transmit_FS("CAN: got here\r\n", 15);
}

void FDCAN::send_message(const fdcan_message_t &msg) {
    FDCAN_HandleTypeDef *hfdcan = (msg.channel == 1) ? &hfdcan1 : &hfdcan2;
    FDCAN_TxHeaderTypeDef tx_header;
    tx_header.Identifier = msg.id;
    tx_header.IdType = msg.isExtended ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
    tx_header.TxFrameType = msg.isRemote ? FDCAN_REMOTE_FRAME : FDCAN_DATA_FRAME;
    tx_header.DataLength = msg.dlc << 16;
    // 10  0000   0000  0000  0000
    tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_header.BitRateSwitch = FDCAN_BRS_OFF;
    tx_header.FDFormat = FDCAN_CLASSIC_CAN;
    tx_header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_header.MessageMarker = 0;

    FDCAN_HandleTypeDef *hfdcan = (msg.channel == 1) ? &hfdcan1 : &hfdcan2;

    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &tx_header, msg.data) == HAL_OK) {
        if (msg.channel == 1) {
            can1_tx_count++;
        } else {
            can2_tx_count++;
        }
    }
}

/* HAL FD CAN Callbacks */

/* CAN Interrupt Callbacks */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    // Debug: Toggle green LED to indicate interrupt was called
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);

    uint8_t channel = 1;
    if (hfdcan == &hfdcan2) {
        channel = 2;
    }
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        receive_can_message(hfdcan,channel);  // Channel 1 for FDCAN1
    }
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        receive_can_message(hfdcan, channel);  // Channel 2 for FDCAN2
    }
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs) {
    // Debug: Toggle green LED to indicate interrupt was called
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    uint8_t channel = 1;
    if (hfdcan == &hfdcan2) {
        channel = 2;
    }
    if((RxFifo1ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        receive_can_message(hfdcan, channel + 2);  // Channel 1 for FDCAN1
    }
    if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        receive_can_message(hfdcan, channel+2);  // Channel 2 for FDCAN1
    }
}

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs) {
    // Optional: Handle transmission events
    // UNUSED(hfdcan);
    uint8_t error_msg[19];

    if (hfdcan == &hfdcan1) {
        // Toggle red LED to indicate CAN1 error
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // Send error message via USB
        memcpy(error_msg, "Retransmit: CAN1\r\n", sizeof(error_msg));
    } else if (hfdcan == &hfdcan2) {
        // Toggle red LED to indicate CAN2 error
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // Send error message via USB
        memcpy(error_msg, "Retransmit: CAN2\r\n", sizeof(error_msg));
    }
    CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
    // Handle CAN errors
    if (hfdcan == &hfdcan1) {
        // Toggle red LED to indicate CAN1 error
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // Send error message via USB
        uint8_t error_msg[] = "CAN1 Error\r\n";
        CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
    } else if (hfdcan == &hfdcan2) {
        // Toggle red LED to indicate CAN2 error
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        // Send error message via USB
        uint8_t error_msg[] = "CAN2 Error\r\n";
        CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
    }
    uint8_t error_msg[19];
    snprintf(reinterpret_cast<char*>(error_msg), sizeof(error_msg),
                                "CAN Error %d\r\n", static_cast<int>(hfdcan->ErrorCode));
    CDC_Transmit_FS(error_msg, sizeof(error_msg) - 1);
}
