/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include "peripheral/iwdg/iwdg.hpp"
#include "main.h"

using HAL::WatchDog;
extern IWDG_HandleTypeDef hiwdg;

void WatchDog::init() {
}

void WatchDog::refresh() {
    HAL_IWDG_Refresh(&hiwdg);
}
