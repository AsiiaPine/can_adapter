/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once

typedef struct {
  uint8_t command;
  uint8_t channel;
  uint8_t id[9];
  uint8_t dlc;
  uint8_t data[8];
} slcan_frame_t;
