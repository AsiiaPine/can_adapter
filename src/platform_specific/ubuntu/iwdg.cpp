/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "peripheral/iwdg/iwdg.hpp"

using HAL::WatchDog;

void WatchDog::init() {
}

void WatchDog::refresh() {
    // No watchdog in Ubuntu platform
}
