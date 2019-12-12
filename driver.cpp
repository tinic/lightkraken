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

void Driver::setsRGBWWCIE(size_t terminal, const rgbww &rgb) {
    terminal %= terminalN;
    _srgbww[terminal] = rgb;
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
            colorConverter[terminal].sRGB8toLEDPWM(
                _srgbww[terminal].r,
                _srgbww[terminal].g,
                _srgbww[terminal].b,
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
            colorConverter[terminal].sRGB8toLEDPWM(
                _srgbww[terminal].r,
                _srgbww[terminal].g,
                _srgbww[terminal].b,
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
            colorConverter[terminal].sRGB8toLEDPWM(
                _srgbww[terminal].r,
                _srgbww[terminal].g,
                _srgbww[terminal].b,
                PwmTimer::pwmPeriod, rp, gp, bp);
            setPulse(0, rp);
            setPulse(1, gp);
            setPulse(2, bp);
            setPulse(3, transferLookup.lookup[_srgbww[terminal].w]);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        uint16_t rp = 0;
        uint16_t gp = 0;
        uint16_t bp = 0;
        colorConverter[terminal].sRGB8toLEDPWM(
            _srgbww[terminal].r,
            _srgbww[terminal].g,
            _srgbww[terminal].b,
            PwmTimer::pwmPeriod, rp, gp, bp);
        setPulse(terminal*3 + 0, rp);
        setPulse(terminal*3 + 1, gp);
        setPulse(terminal*3 + 2, bp);
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
        if (terminal == 0) {
            uint16_t rp = 0;
            uint16_t gp = 0;
            uint16_t bp = 0;
            colorConverter[terminal].sRGB8toLEDPWM(
                _srgbww[terminal].r,
                _srgbww[terminal].g,
                _srgbww[terminal].b,
                PwmTimer::pwmPeriod, rp, gp, bp);
            setPulse(0, rp);
            setPulse(1, gp);
            setPulse(2, bp);
            setPulse(3, transferLookup.lookup[_srgbww[terminal].w]);
        }
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

    RGBColorSpace rgbSpace;
    rgbSpace.setsRGB();
    for (size_t c = 0; c < terminalN; c++) {
        colorConverter[c].setRGBColorSpace(rgbSpace);
    }
    transferLookup.init();

    PwmTimer0::instance().setPulse(0x0);
    PwmTimer1::instance().setPulse(0x0);
    PwmTimer2::instance().setPulse(0x0);
    PwmTimer3::instance().setPulse(0x0);
    PwmTimer5::instance().setPulse(0x0);
    PwmTimer6::instance().setPulse(0x0);
    
    DEBUG_PRINTF(("Driver up.\n"));
}

}
