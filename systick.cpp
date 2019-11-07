/*
Copyright 2019 Tinic Uro

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

}; //extern "C" {

#include "./main.h"
#include "./systick.h"
#include "./status.h"
#include "./model.h"
#include "./strip.h"
#include "./control.h"

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

#ifndef BOOTLOADER
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
#endif  // #ifndef BOOTLOADER

    system_time++;
}

void Systick::init() {
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    SysTick_Config(rcu_clock_freq_get(CK_AHB) / 1000);
    DEBUG_PRINTF(("SysTick up.\n"));
}

}


