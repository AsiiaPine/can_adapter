/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "peripheral/led/led.hpp"
#include "main.h"

using HAL::LED;

void LED::init() {
    HAL_GPIO_WritePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin, GPIO_PIN_SET);
}

void LED::set_color(LEDColor color) {
    switch (color) {
        case LEDColor::LED_RED:
            HAL_GPIO_WritePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin, GPIO_PIN_SET);
            break;
        case LEDColor::LED_GREEN:
            HAL_GPIO_WritePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin, GPIO_PIN_SET);
            break;
        case LEDColor::LED_BLUE:
            HAL_GPIO_WritePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin, GPIO_PIN_RESET);
            break;
        default:
            HAL_GPIO_WritePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin, GPIO_PIN_SET);
            break;
    }
}

void LED::toggle() {
    HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
    HAL_GPIO_TogglePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin);
    HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
}

void LED::toggle(LEDColor color) {
    switch (color) {
        case LEDColor::LED_RED:
            HAL_GPIO_TogglePin(INTERNAL_LED_RED_GPIO_Port, INTERNAL_LED_RED_Pin);
            break;
        case LEDColor::LED_GREEN:
            HAL_GPIO_TogglePin(INTERNAL_LED_GREEN_GPIO_Port, INTERNAL_LED_GREEN_Pin);
            break;
        case LEDColor::LED_BLUE:
            HAL_GPIO_TogglePin(INTERNAL_LED_BLUE_GPIO_Port, INTERNAL_LED_BLUE_Pin);
            break;
        default:
            break;
    }
}
