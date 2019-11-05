/*
* driver.cpp
*
*  Created on: Sep 17, 2019
*      Author: Tinic Uro
*/

#include <stdio.h>
#include <math.h>

extern "C" {
#include "gd32f10x.h"
}

#include "./main.h"
#include "./driver.h"
#include "./pwmtimer.h"
#include "./model.h"

namespace lightkraken {

Driver &Driver::instance() {
    static Driver driver;
    if (!driver.initialized) {
        driver.initialized = true;
        driver.init();
    }
    return driver;
}

void Driver::setRGBWWCIE(size_t terminal, const rgbww &rgb) {
    terminal %= terminalN;
    _rgbww[terminal] = rgb;
}

void Driver::sync(size_t terminal) {
    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        if (terminal == 0) {
			uint16_t rp = 0;
			uint16_t gp = 0;
			uint16_t bp = 0;
        	colorConverter.sRGBtoLEDPWM(
        		_rgbww[terminal].r,
        		_rgbww[terminal].g,
        		_rgbww[terminal].b,
        		PwmTimer::pwmPeriod, rp, gp, bp);
            setPulse(0 + 0, rp);
            setPulse(0 + 1, gp);
            setPulse(0 + 2, bp);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        if (terminal == 0) {
			uint16_t rp = 0;
			uint16_t gp = 0;
			uint16_t bp = 0;
        	colorConverter.sRGBtoLEDPWM(
        		_rgbww[terminal].r,
        		_rgbww[terminal].g,
        		_rgbww[terminal].b,
        		PwmTimer::pwmPeriod, rp, gp, bp);
            setPulse(3 + 0, rp);
            setPulse(0 + 1, gp);
            setPulse(0 + 2, bp);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        if (terminal == 0) {
			uint16_t rp = 0;
			uint16_t gp = 0;
			uint16_t bp = 0;
        	colorConverter.sRGBtoLEDPWM(
        		_rgbww[terminal].r,
        		_rgbww[terminal].g,
        		_rgbww[terminal].b,
        		PwmTimer::pwmPeriod, rp, gp, bp);

            setPulse(0, rp);
            setPulse(1, gp);
            setPulse(2, bp);
            setPulse(3, 0x00);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
		uint16_t rp = 0;
		uint16_t gp = 0;
		uint16_t bp = 0;
		colorConverter.sRGBtoLEDPWM(
			_rgbww[terminal].r,
			_rgbww[terminal].g,
			_rgbww[terminal].b,
			PwmTimer::pwmPeriod, rp, gp, bp);
        setPulse(terminal*3 + 0, rp);
        setPulse(terminal*3 + 1, gp);
        setPulse(terminal*3 + 2, bp);
    } break;
    }
}

void Driver::setPulse(size_t idx, uint16_t pulse) {
    idx %= 6;
    switch(idx) {
    case 0: {
        PwmTimer1::instance().setPulse(pulse);
    } break;
    case 1: {
        PwmTimer2::instance().setPulse(pulse);
    } break;
    case 2: {
        PwmTimer0::instance().setPulse(pulse);
    } break;
    case 3: {
        PwmTimer3::instance().setPulse(pulse);
    } break;
    case 4: {
        PwmTimer5::instance().setPulse(pulse);
    } break;
    case 5: {
        PwmTimer6::instance().setPulse(pulse);
    } break;
    }
}

void Driver::init() {

	colorConverter.setRGBSpace();

    PwmTimer0::instance().setPulse(0x0);
    PwmTimer1::instance().setPulse(0x0);
    PwmTimer2::instance().setPulse(0x0);
    PwmTimer3::instance().setPulse(0x0);
    PwmTimer5::instance().setPulse(0x0);
    PwmTimer6::instance().setPulse(0x0);
    
    DEBUG_PRINTF(("Driver up.\n"));
}

}
