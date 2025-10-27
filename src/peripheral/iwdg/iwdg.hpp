/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

#include <cstdint>

namespace HAL {

class WatchDog {
 public:
    static void init();
    static void refresh();
};

}  // namespace HAL
