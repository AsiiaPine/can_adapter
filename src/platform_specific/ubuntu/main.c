#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "application.hpp"
#include <sys/time.h>


#define UNUSED(x) ((void)(x))

typedef struct {
    uint32_t dummy;
} GPIO_TypeDef;

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    application_entry_point();
    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t PinState) {
    UNUSED(GPIOx);
    UNUSED(GPIO_Pin);
    UNUSED(PinState);
}

void CDC_Transmit_FS(uint8_t *Buf, uint32_t Len) {
    UNUSED(Len);
    printf("%s", Buf);
}

uint32_t HAL_GetTick(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((uint64_t)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

void __enable_irq(void) {
}

void __disable_irq(void) {
}

#ifdef __cplusplus
}
#endif

