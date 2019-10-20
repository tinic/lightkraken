/*
* driver.cpp
*
*  Created on: Sep 17, 2019
*      Author: Tinic Uro
*/

#include <stdio.h>

extern "C" {
#include "gd32f10x.h"
}

#include "./driver.h"

static const uint16_t cie_lookup[256] = {
    0x0000, 0x0004, 0x0007, 0x000b, 0x000e, 0x0012, 0x0015, 0x0019,
    0x001c, 0x0020, 0x0024, 0x0027, 0x002b, 0x002e, 0x0032, 0x0035,
    0x0039, 0x003c, 0x0040, 0x0044, 0x0047, 0x004b, 0x004e, 0x0052,
    0x0056, 0x005a, 0x005e, 0x0063, 0x0067, 0x006c, 0x0070, 0x0075,
    0x007a, 0x007f, 0x0084, 0x008a, 0x008f, 0x0095, 0x009b, 0x00a1,
    0x00a7, 0x00ad, 0x00b4, 0x00ba, 0x00c1, 0x00c8, 0x00cf, 0x00d6,
    0x00de, 0x00e5, 0x00ed, 0x00f5, 0x00fd, 0x0105, 0x010e, 0x0116,
    0x011f, 0x0128, 0x0131, 0x013b, 0x0144, 0x014e, 0x0158, 0x0162,
    0x016c, 0x0177, 0x0182, 0x018c, 0x0198, 0x01a3, 0x01ae, 0x01ba,
    0x01c6, 0x01d2, 0x01df, 0x01eb, 0x01f8, 0x0205, 0x0213, 0x0220,
    0x022e, 0x023c, 0x024a, 0x0258, 0x0267, 0x0276, 0x0285, 0x0295,
    0x02a4, 0x02b4, 0x02c4, 0x02d5, 0x02e5, 0x02f6, 0x0307, 0x0319,
    0x032a, 0x033c, 0x034e, 0x0361, 0x0373, 0x0386, 0x039a, 0x03ad,
    0x03c1, 0x03d5, 0x03e9, 0x03fe, 0x0413, 0x0428, 0x043d, 0x0453,
    0x0469, 0x047f, 0x0496, 0x04ad, 0x04c4, 0x04dc, 0x04f3, 0x050b,
    0x0524, 0x053d, 0x0556, 0x056f, 0x0588, 0x05a2, 0x05bd, 0x05d7,
    0x05f2, 0x060d, 0x0629, 0x0645, 0x0661, 0x067d, 0x069a, 0x06b7,
    0x06d5, 0x06f3, 0x0711, 0x0730, 0x074e, 0x076e, 0x078d, 0x07ad,
    0x07cd, 0x07ee, 0x080f, 0x0830, 0x0852, 0x0874, 0x0896, 0x08b9,
    0x08dc, 0x0900, 0x0923, 0x0948, 0x096c, 0x0991, 0x09b7, 0x09dc,
    0x0a02, 0x0a29, 0x0a50, 0x0a77, 0x0a9f, 0x0ac7, 0x0aef, 0x0b18,
    0x0b41, 0x0b6b, 0x0b95, 0x0bbf, 0x0bea, 0x0c15, 0x0c40, 0x0c6d,
    0x0c99, 0x0cc6, 0x0cf3, 0x0d21, 0x0d4f, 0x0d7d, 0x0dac, 0x0ddc,
    0x0e0b, 0x0e3b, 0x0e6c, 0x0e9d, 0x0ecf, 0x0f01, 0x0f33, 0x0f66,
    0x0f99, 0x0fcd, 0x1001, 0x1035, 0x106a, 0x10a0, 0x10d6, 0x110c,
    0x1143, 0x117a, 0x11b2, 0x11ea, 0x1223, 0x125c, 0x1295, 0x12d0,
    0x130a, 0x1345, 0x1381, 0x13bd, 0x13f9, 0x1436, 0x1473, 0x14b1,
    0x14f0, 0x152e, 0x156e, 0x15ae, 0x15ee, 0x162f, 0x1670, 0x16b2,
    0x16f4, 0x1737, 0x177a, 0x17be, 0x1803, 0x1847, 0x188d, 0x18d3,
    0x1919, 0x1960, 0x19a7, 0x19ef, 0x1a38, 0x1a81, 0x1aca, 0x1b14,
    0x1b5f, 0x1baa, 0x1bf6, 0x1c42, 0x1c8f, 0x1cdc, 0x1d2a, 0x1d78,
    0x1dc7, 0x1e17, 0x1e67, 0x1eb7, 0x1f08, 0x1f5a, 0x1fac, 0x1fff
};

namespace lightguy {

Driver &Driver::instance() {
    static Driver driver;
    if (!driver.initialized) {
        driver.initialized = true;
        driver.init();
    }
    return driver;
}

void Driver::setRGB16(uint16_t r, uint16_t g, uint16_t b) {
    setPulse(1, r>>3);
    setPulse(2, g>>3);
    setPulse(0, b>>3);
}

void Driver::setRGBW16(uint16_t r, uint16_t g, uint16_t b, uint16_t w) {
    setPulse(1, r>>3);
    setPulse(2, g>>3);
    setPulse(0, b>>3);
    setPulse(3, w>>3);
}

void Driver::setRGB8CIE(uint8_t r, uint8_t g, uint8_t b) {
    setPulse(1, cie_lookup[r]);
    setPulse(2, cie_lookup[g]);
    setPulse(0, cie_lookup[b]);
}

void Driver::setRGBW8CIE(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    setPulse(1, cie_lookup[r]);
    setPulse(2, cie_lookup[g]);
    setPulse(0, cie_lookup[b]);
    setPulse(3, cie_lookup[w]);
}

void Driver::setPulse(size_t idx, uint16_t pulse) {
    (void)idx;
    (void)pulse;
#if 0
    idx %= 4;

    if (pulse > 0x1ffe) {
        pulse = 0x1ffe;
    }

    switch(idx) {
    case 0: {
        static bool init = false;
        static bool started = false;
        if (pulse < 0x0002) {
            if (started) HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
            started = false;
            init = false;
        } else {
            if (!init) {
                init = true;
                TIM_OC_InitTypeDef sConfigOC;
                memset(&sConfigOC, 0, sizeof(sConfigOC));
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = pulse;
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
                sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
                HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);
            } else {
                htim1.Instance->CCR3 = pulse;
            }
            if (!started) HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
            started = true;
        }
    } break;
    case 1: {
        static bool init = false;
        static bool started = false;
        if (pulse < 0x0002) {
            if (started) HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
            started = false;
            init = false;
        } else {
            if (!init) {
                init = true;
                TIM_OC_InitTypeDef sConfigOC;
                memset(&sConfigOC, 0, sizeof(sConfigOC));
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = pulse;
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
            } else {
                htim2.Instance->CCR1 = pulse;
            }
            if (!started) HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
            started = true;
        }
    } break;
    case 2: {
        static bool init = false;
        static bool started = false;
        if (pulse < 0x0002) {
            if (started) HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            started = false;
            init = false;
        } else {
            if (!init) {
                init = true;
                TIM_OC_InitTypeDef sConfigOC;
                memset(&sConfigOC, 0, sizeof(sConfigOC));
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = pulse;
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
            } else {
                htim3.Instance->CCR2 = pulse;
            }
            if (!started) HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            started = true;
        }
    } break;
    case 3: {
        static bool init = false;
        static bool started = false;
        if (pulse < 0x0002) {
            if (started) HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
            started = false;
            init = false;
        } else {
            if (!init) {
                init = true;
                TIM_OC_InitTypeDef sConfigOC;
                memset(&sConfigOC, 0, sizeof(sConfigOC));
                sConfigOC.OCMode = TIM_OCMODE_PWM1;
                sConfigOC.Pulse = pulse;
                sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
                sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
                HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
            } else {
                htim4.Instance->CCR4 = pulse;
            }
            if (!started) HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
            started = true;
        }
    } break;
    }
#endif  // #if 0
}

void Driver::init() {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    
    gpio_bit_set(GPIOA, GPIO_PIN_0);
    gpio_bit_set(GPIOA, GPIO_PIN_10);
    gpio_bit_set(GPIOB, GPIO_PIN_7);
    gpio_bit_set(GPIOB, GPIO_PIN_8);
    gpio_bit_set(GPIOB, GPIO_PIN_9);
    gpio_bit_set(GPIOC, GPIO_PIN_7);
    
    printf("Driver up.\n");
}

}
