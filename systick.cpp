#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
#include "cmsis_gcc.h"

void SysTick_Handler(void);

}; //extern "C" {

#include "./systick.h"
#include "./status.h"

void SysTick_Handler(void) {
	lightguy::Systick::instance().handler();
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
    system_time++;
}

void Systick::init() {
    systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);
    SysTick_Config(rcu_clock_freq_get(CK_AHB) / 1000); 

    printf("SysTick up.\n");
    
}

}
