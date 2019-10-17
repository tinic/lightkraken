#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
}

#include "./status.h"

namespace lightguy {

enum PowerClass {
	PSE_TYPE_1_2_CLASS_0_3  = 0b111,
	PSE_TYPE_2_CLASS_4      = 0b101,
	PSE_TYPE_3_4_CLASS_0_3  = 0b110,
	PSE_TYPE_3_4_CLASS_4    = 0b100,
	PSE_TYPE_3_4_CLASS_5_6  = 0b010,
	PSE_TYPE_4_CLASS_7_8    = 0b000,
};

StatusLED &StatusLED::instance() {
	static StatusLED status_led;
	if (!status_led.initialized) {
		status_led.initialized = true;
		status_led.init();
	}
	return status_led;
}

void StatusLED::init() {
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);    
    gpio_bit_reset(GPIOB, GPIO_PIN_6);

    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);

	update();

    printf("Status LED up.");
}

void StatusLED::update() {
	readPowerState();
	if (!powergood_state) {
		setUserLED(0x1f, 0x00, 0x00);
		return;
	}
	PowerClass pclass = (PowerClass)(((tph_state)?0x4:0x0)|
								 	 ((tpl_state)?0x2:0x0)|
									 (( bt_state)?0x0:0x1));
	switch(pclass) {
		default:
		setUserLED(0x00, 0x00, 0xff);
		break;
		case PSE_TYPE_3_4_CLASS_0_3:
		case PSE_TYPE_1_2_CLASS_0_3:
		setUserLED(0x1f, 0x0f, 0x00);
		break;
		case PSE_TYPE_3_4_CLASS_4:
		case PSE_TYPE_2_CLASS_4:
		setUserLED(0x1f, 0x1f, 0x00);
		break;
		case PSE_TYPE_3_4_CLASS_5_6:
		case PSE_TYPE_4_CLASS_7_8:
		setUserLED(0x00, 0x1f, 0x00);
		break;
	}
}

__attribute__ ((hot, optimize("O2")))
void StatusLED::setUserLED(uint8_t r, uint8_t g, uint8_t b) {
	constexpr int32_t one = 20; // for 108Mhz
	constexpr int32_t t0l = (one * 35) / 130;
	constexpr int32_t t0h = (one * 95) / 130;
	constexpr int32_t t1l = (one * 70) / 130;
	constexpr int32_t t1h = (one * 60) / 130;
	uint32_t bits = (uint32_t(g)<<16) | (uint32_t(r)<<8) | uint32_t(b);
	for (int32_t d=23; d>=0; d--) {
		if ((1UL<<d) & bits) {
			// one
			for (int32_t c =0 ; c<t1l; c++) {
              GPIO_BOP(GPIOB) = GPIO_PIN_6;
			}
			for (int32_t c =0 ; c<t1h; c++) {
              GPIO_BC(GPIOB) = GPIO_PIN_6;
			}
		} else {
			// zero
			for (int32_t c =0 ; c<t0l; c++) {
              GPIO_BOP(GPIOB) = GPIO_PIN_6;
			}
			for (int32_t c =0 ; c<t0h; c++) {
              GPIO_BC(GPIOB) = GPIO_PIN_6;
			}
		}
	}
    GPIO_BC(GPIOB) = GPIO_PIN_6;
}

void StatusLED::readPowerState() {
	bt_state = gpio_input_bit_get(GPIOA, GPIO_PIN_3) == RESET ? false : true;
	tpl_state = gpio_input_bit_get(GPIOA, GPIO_PIN_5) == RESET ? false : true;
	tph_state = gpio_input_bit_get(GPIOA, GPIO_PIN_4) == RESET ? false : true;
	powergood_state = gpio_input_bit_get(GPIOB, GPIO_PIN_0) == RESET ? false : true;
}

}  // namespace lightguy {
