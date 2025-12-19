/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */


#include "main.h"
#include "usbd_cdc_if.h"

int _write(int file, char *ptr, int len) {
    (void)file;
    CDC_Send(0, ptr, len);

    return len;
}
