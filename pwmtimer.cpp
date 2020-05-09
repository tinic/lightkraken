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
}

#include "./main.h"
#include "./pwmtimer.h"

namespace lightkraken {
    
// BLUE
PwmTimer &PwmTimer0::instance() {
    static PwmTimer0 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }
    return timer;
}

void PwmTimer0::init() {
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = pwmPeriod;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0, &timer_initpara);

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER0, TIMER_CH_2, &timer_ocintpara);

    timer_break_parameter_struct timer_breakpara;
    timer_breakpara.runoffstate     = TIMER_ROS_STATE_DISABLE;
    timer_breakpara.ideloffstate    = TIMER_IOS_STATE_DISABLE;
    timer_breakpara.deadtime        = 0;
    timer_breakpara.breakpolarity   = TIMER_BREAK_POLARITY_HIGH;
    timer_breakpara.outputautostate = TIMER_OUTAUTO_DISABLE;
    timer_breakpara.protectmode     = TIMER_CCHP_PROT_OFF;
    timer_breakpara.breakstate      = TIMER_BREAK_DISABLE;
    timer_break_config(TIMER0, &timer_breakpara);

    timer_channel_output_fast_config(TIMER0, TIMER_CH_2, TIMER_OC_FAST_ENABLE);

    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, 0);
    timer_channel_output_mode_config(TIMER0, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);
    timer_master_slave_mode_config(TIMER0, TIMER_MASTER_SLAVE_MODE_DISABLE);

    timer_primary_output_config(TIMER0, ENABLE);

    timer_auto_reload_shadow_disable(TIMER0);
    timer_disable(TIMER0);
}

void PwmTimer0::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_disable(TIMER0);
        gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    } else {
        if (pulse > TIMER_CAR(TIMER0)-1) {
            pulse = TIMER_CAR(TIMER0)-1;
        }
        gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
        timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_2, pulse);
        timer_enable(TIMER0);
    }
}
    
PwmTimer &PwmTimer1::instance() {
    static PwmTimer1 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }        
    return timer;
}

// RED
void PwmTimer1::init() {
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_TIMER1);

    timer_deinit(TIMER1);
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = pwmPeriod;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1,&timer_initpara);

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1, TIMER_CH_0, &timer_ocintpara);
    
    timer_channel_output_fast_config(TIMER1, TIMER_CH_0, TIMER_OC_FAST_ENABLE);

    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 0);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    timer_primary_output_config(TIMER1, ENABLE);
    
    timer_auto_reload_shadow_disable(TIMER1);
    timer_disable(TIMER1);
}

void PwmTimer1::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_disable(TIMER1);
        gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    } else {
        if (pulse > TIMER_CAR(TIMER1)-1) {
            pulse = TIMER_CAR(TIMER1)-1;
        }
        gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
        timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, pulse);
        timer_enable(TIMER1);
    }
}

// GREEN
PwmTimer &PwmTimer2::instance() {
    static PwmTimer2 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }
    return timer;
}

void PwmTimer2::init() {
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_TIMER2);

    gpio_pin_remap_config(GPIO_TIMER2_FULL_REMAP, ENABLE);
    
    timer_deinit(TIMER2);
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = pwmPeriod;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER2, &timer_initpara);

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER2, TIMER_CH_1, &timer_ocintpara);

    timer_channel_output_fast_config(TIMER2, TIMER_CH_1, TIMER_OC_FAST_ENABLE);

    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 0);
    timer_channel_output_mode_config(TIMER2, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER2, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    timer_primary_output_config(TIMER2, ENABLE);
    
    timer_auto_reload_shadow_disable(TIMER2);
    timer_disable(TIMER2);
}

void PwmTimer2::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_disable(TIMER2);
        gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    } else {
        if (pulse > TIMER_CAR(TIMER2)-1) {
            pulse = TIMER_CAR(TIMER2)-1;
        }
        gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
        timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, pulse);
        timer_enable(TIMER2);
    }
}

PwmTimer &PwmTimer3::instance() {
    static PwmTimer3 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }
    return timer;
}

// WHITE A (RED)
void PwmTimer3::init() {
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_TIMER3);

    timer_deinit(TIMER3);
    timer_parameter_struct timer_initpara;
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = pwmPeriod;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3, &timer_initpara);

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER3, TIMER_CH_2, &timer_ocintpara);

    timer_break_parameter_struct timer_breakpara;
    timer_breakpara.runoffstate     = TIMER_ROS_STATE_DISABLE;
    timer_breakpara.ideloffstate    = TIMER_IOS_STATE_DISABLE;
    timer_breakpara.deadtime        = 0;
    timer_breakpara.breakpolarity   = TIMER_BREAK_POLARITY_HIGH;
    timer_breakpara.outputautostate = TIMER_OUTAUTO_DISABLE;
    timer_breakpara.protectmode     = TIMER_CCHP_PROT_OFF;
    timer_breakpara.breakstate      = TIMER_BREAK_DISABLE;
    timer_break_config(TIMER3, &timer_breakpara);

    timer_channel_output_fast_config(TIMER3, TIMER_CH_2, TIMER_OC_FAST_ENABLE);

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_2, 0);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_2, TIMER_OC_SHADOW_DISABLE);
    timer_master_slave_mode_config(TIMER3, TIMER_MASTER_SLAVE_MODE_DISABLE);

    timer_primary_output_config(TIMER3, ENABLE);
    
    timer_auto_reload_shadow_disable(TIMER3);
    timer_disable(TIMER3);    
}

void PwmTimer3::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_disable(TIMER3);
        gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    } else {
        if (pulse > TIMER_CAR(TIMER3)-1) {
            pulse = TIMER_CAR(TIMER3)-1;
        }
        gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_2, pulse);
        timer_enable(TIMER3);
    }
}

// WHITE B (GREEN)
PwmTimer &PwmTimer5::instance() {
    static PwmTimer5 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }
    return timer;
}

void PwmTimer5::init() {
    // TIMER3 CH1
    PwmTimer3::instance();

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER3, TIMER_CH_1, &timer_ocintpara);
    
    timer_channel_output_fast_config(TIMER3, TIMER_CH_1, TIMER_OC_FAST_ENABLE);
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_1, 0);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);
}

void PwmTimer5::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_channel_output_mode_config(TIMER3, TIMER_CH_1, TIMER_OC_MODE_LOW);
    } else {
        if (pulse > TIMER_CAR(TIMER3)-1) {
            pulse = TIMER_CAR(TIMER3)-1;
        }
        timer_channel_output_mode_config(TIMER3, TIMER_CH_1, TIMER_OC_MODE_PWM0);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_1, pulse);
        timer_enable(TIMER3);
    }
}

// WHITE C (BLUE)
PwmTimer &PwmTimer6::instance() {
    static PwmTimer6 timer;
    if (!timer.initialized) {
        timer.initialized = true;
        timer.init();
    }
    return timer;
}

void PwmTimer6::init() {
    PwmTimer3::instance();

    timer_oc_parameter_struct timer_ocintpara;
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_ENABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER3, TIMER_CH_3, &timer_ocintpara);
    
    timer_channel_output_fast_config(TIMER3, TIMER_CH_3, TIMER_OC_FAST_ENABLE);
    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, 0);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_3, TIMER_OC_SHADOW_DISABLE);
}

void PwmTimer6::setPulse(uint16_t pulse) {
    if (pulse < 2) {
        timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_LOW);
    } else {
        if (pulse > TIMER_CAR(TIMER3)-1) {
            pulse = TIMER_CAR(TIMER3)-1;
        }
        timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_PWM0);
        timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, pulse);
        timer_enable(TIMER3);
    }
}

} // namespace lightkraken
