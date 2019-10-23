#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

}; //extern "C" {

#include "./main.h"
#include "./systick.h"
#include "./status.h"

extern "C" {
__attribute__((used)) // required for -flto
void SysTick_Handler(void) {
    lightguy::Systick::instance().handler();
}
}

namespace lightguy {

Systick &Systick::instance() {
    static Systick systick;
    if (!systick.initialized) {
        systick.initialized = true;
        systick.init();
    }
    return systick;
}

void Systick::handler() {
    static uint32_t status_led = 0;
    if ((status_led++ & 0xF) == 0x0) {
        lightguy::StatusLED::instance().schedule();
    }
    if (nvic_reset_delay > 0) {
        nvic_reset_delay--;
    }
    if (nvic_reset_delay == 1) {
#ifdef BOOTLOADER
        lightguy::StatusLED::instance().setBootloaderStatus(lightguy::StatusLED::reset);
#endif  // #ifdef BOOTLOADER
        if (bootloader_after_reset) {
            *((volatile uint32_t *)0x20000000) = 0xFEEDC0DE;
        }
        NVIC_SystemReset();
    }

    system_time++;
}

void Systick::init() {
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    SysTick_Config(rcu_clock_freq_get(CK_AHB) / 1000);
    DEBUG_PRINTF(("SysTick up.\n"));
}

}


