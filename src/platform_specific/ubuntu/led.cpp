/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "peripheral/led/led.hpp"
#include "main.h"

using HAL::LED;

bool led_color[3] = {false, false, false};

void LED::init() {
    for (int i = 0; i < 3; i++) {
        led_color[i] = false;
    }
}

void LED::set_color(LEDColor color) {
    if (color == LEDColor::LED_OFF) {
        return;
    }
    led_color[static_cast<int>(color)] = true;
}

void LED::toggle() {
}

void LED::toggle(LEDColor color) {
    if (color == LEDColor::LED_OFF) {
        return;
    }
    led_color[static_cast<int>(color)] = !led_color[static_cast<int>(color)];
}
