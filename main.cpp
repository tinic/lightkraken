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
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"
}; //extern "C" {

#include "./main.h"
#include "./netconf.h"
#include "./strip.h"
#include "./model.h"
#include "./status.h"
#include "./control.h"

#ifdef BOOTLOADER
#include "./bootloader.h"
  
typedef  void (*pFunction)(void);
static pFunction Jump_To_Application;
static uint32_t JumpAddress;
#endif  // #ifdef BOOTLOADER

int main() {

#ifdef BOOTLOADER
    // Check user button
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
    enet_delay(100);
    if (gpio_input_bit_get(GPIOB, GPIO_PIN_1) != RESET && *((volatile uint32_t *)0x20000000) != 0xFEEDC0DE) {
        *((volatile uint32_t *)0x20000000) = 0;
        /* Check if valid stack address (RAM address) then jump to user application */
        if (((*(__IO uint32_t*)USER_FLASH_FIRST_PAGE_ADDRESS) & 0x2FFE0000 ) == 0x20000000) {
            /* Jump to user application */
            JumpAddress = *(__IO uint32_t*) (USER_FLASH_FIRST_PAGE_ADDRESS + 4);
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP(*(__IO uint32_t*) USER_FLASH_FIRST_PAGE_ADDRESS);
            Jump_To_Application();
        }
    }
    *((volatile uint32_t *)0x20000000) = 0;
#endif  // #ifdef BOOTLOADER

    nvic_vector_table_set(NVIC_BASE_ADDRESS,0);

    while (1) {
        lightkraken::NetConf::instance().update();
        lightkraken::StatusLED::instance().update();
        
#ifndef BOOTLOADER
		lightkraken::Control::instance().update();
#endif  //#ifndef BOOTLOADER
    }
    return 0;
}
