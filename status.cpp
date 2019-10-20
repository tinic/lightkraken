#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
}

#include "./status.h"

namespace lightguy {

StatusLED &StatusLED::instance() {
    static StatusLED status_led;
    if (!status_led.initialized) {
        status_led.initialized = true;
        status_led.init();
    }
    return status_led;
}

void StatusLED::init() {
    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);    
    gpio_bit_reset(GPIOB, GPIO_PIN_6);

    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
}

void StatusLED::update() {

    if (!scheduled) {
        return;
    }
    
    scheduled = false;

    readPowerState();

    PowerClass pclass = PSE_TYPE_INVALID;
    if (!powergood_state) {
    pclass = PSE_TYPE_POWER_BAD;
    } else {
    pclass = (PowerClass)(((tph_state)?0x4:0x0)|
                            ((tpl_state)?0x2:0x0)|
                            (( bt_state)?0x0:0x1));
    }
    if (power_class != pclass) {
        power_class = pclass;
        switch(power_class) {
        default:
            setUserLED(0x00, 0x00, 0xff);
            break;
        case PSE_TYPE_3_4_CLASS_0_3:
        case PSE_TYPE_1_2_CLASS_0_3:
            printf("POE Power Class 0-3 (0-12.5W)\n");
            setUserLED(0x1f, 0x0f, 0x00);
            break;
        case PSE_TYPE_3_4_CLASS_4:
        case PSE_TYPE_2_CLASS_4:
            printf("POE Power Class 4 (0-25W)\n");
            setUserLED(0x1f, 0x1f, 0x00);
            break;
        case PSE_TYPE_3_4_CLASS_5_6:
            printf("POE Power Class 5-6 (0-50W)\n");
            setUserLED(0x0f, 0x1f, 0x00);
            break;
        case PSE_TYPE_4_CLASS_7_8:
            printf("POE Power Class 7-8 (0-70W)\n");
            setUserLED(0x00, 0x1f, 0x00);
            break;
        }
    }
}

__attribute__ ((hot, optimize("O2")))
void StatusLED::setUserLED(uint8_t r, uint8_t g, uint8_t b) {

    __disable_irq();
    
    uint32_t bits = (uint32_t(g)<<16) | (uint32_t(r)<<8) | uint32_t(b);
    for (int32_t d=23; d>=0; d--) {
#define SET GPIO_BOP(GPIOB) = GPIO_PIN_6;
#define RST GPIO_BC(GPIOB) = GPIO_PIN_6;
    if ((1UL<<d) & bits) {
            // one
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 

            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;

            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
        } else {
            // zero
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            SET; SET; SET; SET; 
            
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;

            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;

            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
            RST; RST; RST; RST;
        }
    }
    
    GPIO_BC(GPIOB) = GPIO_PIN_6;

    __enable_irq();
}

void StatusLED::readPowerState() {
    bt_state = gpio_input_bit_get(GPIOA, GPIO_PIN_3) == RESET ? false : true;
    tpl_state = gpio_input_bit_get(GPIOA, GPIO_PIN_5) == RESET ? false : true;
    tph_state = gpio_input_bit_get(GPIOA, GPIO_PIN_4) == RESET ? false : true;
    powergood_state = gpio_input_bit_get(GPIOB, GPIO_PIN_0) == RESET ? false : true;
}

}  // namespace lightguy {
