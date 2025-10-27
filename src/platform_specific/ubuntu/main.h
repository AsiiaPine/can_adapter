/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t HAL_GetTick(void);
void HAL_GPIO_WritePin(void *GPIOx, uint16_t GPIO_Pin, uint16_t PinState);
void CDC_Transmit_FS(uint8_t *Buf, uint32_t Len);
void __disable_irq(void);
void __enable_irq(void);

#ifdef __cplusplus
}
#endif
