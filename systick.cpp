#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

}; //extern "C" {

#include "./main.h"
#include "./systick.h"
#include "./status.h"
#include "./model.h"

extern "C" {
__attribute__((used)) // required for -flto
void SysTick_Handler(void) {
    lightkraken::Systick::instance().handler();
}
}

namespace lightkraken {

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
        lightkraken::StatusLED::instance().schedule();
    }
    if (nvic_reset_delay > 0) {
        nvic_reset_delay--;
    }
    if (nvic_reset_delay == 1) {
#ifdef BOOTLOADER
        lightkraken::StatusLED::instance().setBootloaderStatus(lightkraken::StatusLED::reset);
#endif  // #ifdef BOOTLOADER
        if (bootloader_after_reset) {
            *((volatile uint32_t *)0x20000000) = 0xFEEDC0DE;
        }
        NVIC_SystemReset();
    }
    
    if (apply_scheduled) {
        if (StatusLED::instance().enetUp()) {
            StatusLED::PowerClass powerClass = StatusLED::instance().powerClass();
            if ( powerClass == StatusLED::PSE_TYPE_1_2_CLASS_0_3 ||
                 powerClass == StatusLED::PSE_TYPE_2_CLASS_4 ||
                 powerClass == StatusLED::PSE_TYPE_3_4_CLASS_0_3 ||
                 powerClass == StatusLED::PSE_TYPE_3_4_CLASS_0_3 ||
                 powerClass == StatusLED::PSE_TYPE_3_4_CLASS_4 ||
                 powerClass == StatusLED::PSE_TYPE_3_4_CLASS_5_6 ||
                 powerClass == StatusLED::PSE_TYPE_4_CLASS_7_8 ) {

                Model::instance().apply();
                apply_scheduled = false;
            }
        }
    }

    system_time++;
}

void Systick::init() {
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    SysTick_Config(rcu_clock_freq_get(CK_AHB) / 1000);
    DEBUG_PRINTF(("SysTick up.\n"));
}

}


