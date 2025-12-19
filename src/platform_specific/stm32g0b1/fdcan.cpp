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
#include "peripheral/usb/usb.hpp"

using HAL::FDCAN;
using HAL::FDCANChannel;
using HAL::fdcan_message_t;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

#define MAX_MESSAGES 10

uint8_t HAL::FDCAN::status[2] = {0};
static uint8_t get_message_length(FDCAN_RxHeaderTypeDef *rx_header);


fdcan_message_t FDCAN::buffer[2][MAX_MESSAGES] = {{}};

MessagesCircularBuffer<fdcan_message_t> FDCAN::messages[2] = {
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[0], TIM16_FDCAN_IT0_IRQn),
    MessagesCircularBuffer<fdcan_message_t>(MAX_MESSAGES, FDCAN::buffer[1], TIM16_FDCAN_IT0_IRQn)
};

uint32_t standart_bitrate = 1000000;
FDCAN_InitTypeDef standart_settings = hfdcan1.Init;

int8_t FDCAN::stop(FDCANChannel channel) {
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;
    return HAL_FDCAN_Stop(hfdcan);
}

int8_t FDCAN::start(FDCANChannel channel) {
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;
    return HAL_FDCAN_Start(hfdcan);
}

int8_t FDCAN::receive_message(HAL::FDCANChannel channel, HAL::fdcan_message_t& msg) {
    if (channel >= FDCANChannel::NUM_CHANNELS) {
        return -1;
    }
    if (FDCAN::messages[channel].pop_message(&msg) < 0) {
        return -1;
    }
    return 0;
}

void FDCAN::send_message(fdcan_message_t *msg) {
    FDCAN_HandleTypeDef *hfdcan = (msg->channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;
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
        msg.id = rx_header->Identifier & 0x1FFFFFFF;
    } else {
        msg.id = rx_header->Identifier & 0x7FF;
    }

    // Skip messages with invalid ID
    if (msg.id == 0) {
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
        return;
    }

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

    msg.timestamp = HAL_GetTick() % 60000;
    // Only push valid messages
    HAL::FDCAN::messages[channel].push_message(msg);
}

int8_t FDCAN::set_bitrate(uint8_t channel, uint32_t bitrate) {
    if (channel >= FDCANChannel::NUM_CHANNELS) {
        return -1;
    }
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;

    while (HAL_FDCAN_GetState(hfdcan) != HAL_FDCAN_STATE_READY) {
        // Wait until the FDCAN is ready
    }
    HAL_FDCAN_DeInit(hfdcan);
    uint32_t clock_divider = standart_bitrate / bitrate;

    hfdcan->Init.ClockDivider = clock_divider * standart_settings.NominalPrescaler;

    hfdcan->Init.NominalSyncJumpWidth = standart_settings.NominalSyncJumpWidth;
    hfdcan->Init.NominalTimeSeg1 = standart_settings.NominalTimeSeg1;
    hfdcan->Init.NominalTimeSeg2 = standart_settings.NominalTimeSeg2;
    hfdcan->Init.DataPrescaler = standart_settings.DataPrescaler;
    hfdcan->Init.DataSyncJumpWidth = standart_settings.DataSyncJumpWidth;
    hfdcan->Init.DataTimeSeg1 = standart_settings.DataTimeSeg1;
    hfdcan->Init.DataTimeSeg2 = standart_settings.DataTimeSeg2;
    return HAL_FDCAN_Init(hfdcan);
}

int8_t FDCAN::set_custom_bitrate(uint8_t channel, uint8_t time_quantum, uint8_t jump_width,
                                    uint8_t time_segment1, uint8_t time_segment2) {
    if (channel >= FDCANChannel::NUM_CHANNELS) {
        return -1;
    }
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;

    while (HAL_FDCAN_GetState(hfdcan) != HAL_FDCAN_STATE_READY) {
        // Wait until the FDCAN is ready
    }
    HAL_FDCAN_DeInit(hfdcan);

    hfdcan->Init.NominalPrescaler = time_quantum;  // Configure according to your bitrate requirements
    hfdcan->Init.NominalSyncJumpWidth = jump_width;
    hfdcan->Init.NominalTimeSeg1 = time_segment1;
    hfdcan->Init.NominalTimeSeg2 = time_segment2;

    // Optional: configure Data settings similarly if needed
    hfdcan->Init.DataPrescaler = 1;  // Depending on requirements
    hfdcan->Init.DataSyncJumpWidth = 1;
    hfdcan->Init.DataTimeSeg1 = 1;
    hfdcan->Init.DataTimeSeg2 = 1;

    return HAL_FDCAN_Init(hfdcan);
}


void FDCAN::check_can_bus(uint8_t channel) {
    FDCAN_HandleTypeDef *hfdcan = (channel == FDCANChannel::CHANNEL_1) ? &hfdcan1 : &hfdcan2;
    FDCAN_ProtocolStatusTypeDef protocolStatus = {};

    HAL_FDCAN_GetProtocolStatus(hfdcan, &protocolStatus);
    if (protocolStatus.BusOff) {
        CLEAR_BIT(hfdcan->Instance->CCCR, FDCAN_CCCR_INIT);
    }

    if (protocolStatus.ErrorPassive) {
        status[channel] |= 1 << FDCAN_STATUS_BITS::ERROR_PASSIVE;
    }
    if (protocolStatus.Warning) {
        status[channel] |= 1 << FDCAN_STATUS_BITS::ERROR_WARNING;
    }
    if (protocolStatus.BusOff) {
        status[channel] |= 1 << FDCAN_STATUS_BITS::BUS_ERROR;
    }
    char buf[5];
    snprintf(buf, sizeof(buf), "F%02x\r\n", static_cast<int>(status[channel]));
    HAL::USB::send_message(reinterpret_cast<uint8_t*>(buf), strlen(buf), channel);
}

/* ===================================Callbacks==================================== */

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    // Debug: Toggle green LED to indicate interrupt was called
    uint8_t channel = hfdcan == &hfdcan1 ? FDCANChannel::CHANNEL_1 : FDCANChannel::CHANNEL_2;
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL == RESET) {
        // RX FIFO 0 is full
        HAL::FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::RX_FIFO_FULL;
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
    }
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == RESET) {
        return;
    }
    if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) {
        // Message lost
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
    }
    uint8_t rx_data[64];
    FDCAN_RxHeaderTypeDef rx_header;
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        return;
    }
    HAL_GPIO_TogglePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin);
    push_can_message(channel, &rx_header, rx_data);
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs) {
    // Debug: Toggle blue LED to indicate interrupt was called
    HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
    uint8_t channel = hfdcan == &hfdcan1 ? FDCANChannel::CHANNEL_1 : FDCANChannel::CHANNEL_2;
    if (RxFifo1ITs & FDCAN_IT_RX_FIFO1_FULL) {
        // RX FIFO 0 is full
        HAL::FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::RX_FIFO_FULL;
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
    }
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) == RESET) {
        return;
    }
    if (RxFifo1ITs & FDCAN_IT_RX_FIFO1_MESSAGE_LOST) {
        // Message lost
        HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
    }
    uint8_t rx_data[64];
    FDCAN_RxHeaderTypeDef rx_header;
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        return;
    }
    push_can_message(channel, &rx_header, rx_data);
}

void HAL_FDCAN_TxEventFifoCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs) {
    // Optional: Handle transmission events
    uint8_t error_msg[19];
    FDCANChannel channel = hfdcan == &hfdcan1 ? FDCANChannel::CHANNEL_1 : FDCANChannel::CHANNEL_2;

    if (TxEventFifoITs & FDCAN_IT_TX_EVT_FIFO_FULL) {
        // Transmission event FIFO is full
        FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::TX_FIFO_FULL;
    }
    if (TxEventFifoITs & FDCAN_IT_TX_EVT_FIFO_ELT_LOST) {
        // Transmission event lost
    }
    if (TxEventFifoITs & FDCAN_IT_TX_EVT_FIFO_NEW_DATA) {
        // Transmission event occurred
    }
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
    FDCANChannel channel = hfdcan == &hfdcan1 ? FDCANChannel::CHANNEL_1 : FDCANChannel::CHANNEL_2;
    // Handle CAN errors
    if (hfdcan->ErrorCode && FDCAN_IT_RAM_ACCESS_FAILURE) {
        // Critical error: RAM access failure
        HAL::FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::DATA_OVERRUN;
    }
    if (hfdcan->ErrorCode && FDCAN_IT_ARB_PROTOCOL_ERROR) {
        // Bus off error
        HAL::FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::ARBITRATION_LOST;
    }

    uint8_t error_msg[19];
    snprintf(reinterpret_cast<char*>(error_msg), sizeof(error_msg),
                                "CAN Error %d\r\n", static_cast<int>(hfdcan->ErrorCode));
    HAL::USB::send_message(error_msg, sizeof(error_msg) - 1, channel);
    FDCAN::status[channel] = 0;
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
    FDCANChannel channel = hfdcan == &hfdcan1 ? FDCANChannel::CHANNEL_1 : FDCANChannel::CHANNEL_2;
    FDCAN::status[channel] = 0;
    if ((ErrorStatusITs & FDCAN_IR_EP) != 0) {
        FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::ERROR_PASSIVE;
    }
    if ((ErrorStatusITs & FDCAN_IR_EW) != 0) {
        FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::ERROR_WARNING;
    }
    if ((ErrorStatusITs & FDCAN_IR_BO) != 0) {
        FDCAN::status[channel] |= 1 << HAL::FDCAN_STATUS_BITS::BUS_ERROR;
    }
}

/* ===================================Helper functions==================================== */

/**
    * @brief    Gets message length from FDCAN_RxHeaderTypeDef
    * @note     Helper function to send message via USB
    * @param    *rx_header: FDCAN_RxHeaderTypeDef FDCAN Rx Header
    * @retval   uint8_t - Message length, returns 255 if RxHeader is null or invalid
*/ 
static uint8_t get_message_length(FDCAN_RxHeaderTypeDef *rx_header) {
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

// Took from https://www.systemonchips.com/stm32h7-can-fd-communication-errors-and-troubleshooting-guide/
void FDCAN::PrintCANStatus(void) {
    uint32_t status = hfdcan1.Instance->PSR;
    uint8_t status_bits = 0;
    printf("CAN Status:\n");
    if (status & FDCAN_PSR_EP) status_bits |= (1 << FDCAN_STATUS_BITS::ERROR_PASSIVE);
    if (status & FDCAN_PSR_EW) status_bits |= (1 << FDCAN_STATUS_BITS::ERROR_WARNING);
    if (status & FDCAN_PSR_BO) status_bits |= (1 << FDCAN_STATUS_BITS::BUS_ERROR);
    printf(" Status: 0x%02X\n", status_bits);
    printf("  Last Error Code: %lu\n", (status & FDCAN_PSR_LEC) >> FDCAN_PSR_LEC_Pos);
    printf("  Activity: %s\n", (status & FDCAN_PSR_ACT) ? "Active" : "Inactive");
    printf("  Error Passive: %s\n", (status & FDCAN_PSR_EP) ? "Yes" : "No");
    printf("  Warning Status: %s\n", (status & FDCAN_PSR_EW) ? "Yes" : "No");
    printf("  Bus Off: %s\n", (status & FDCAN_PSR_BO) ? "Yes" : "No");
}
