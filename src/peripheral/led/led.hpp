/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include <cstdint>

namespace HAL {

enum LEDColor: uint8_t {
  LED_RED = 0,
  LED_GREEN = 1,
  LED_BLUE = 2,
  LED_OFF = 4,
};

class LED {
 public:
    static void init();
    static void set_color(LEDColor color);
    static void toggle();
    static void toggle(LEDColor color);
};

}  // namespace HAL
