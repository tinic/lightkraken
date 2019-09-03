#include <stdint.h>

#ifdef STM32F1xx
#include "stm32f1xx.h"
#endif  // #ifdef STM32F103xB

int main() {
    while (1) {
        __WFI();
    }
    return 0;
}
