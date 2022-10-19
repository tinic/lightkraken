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

void Driver::setRGBWW(size_t terminal, const rgbww &rgb) {
    terminal %= terminalN;
    _srgbww[terminal] = rgb;
}

void Driver::sync(size_t terminal) {
    auto convert = [=, this] (const rgbww &rgb) {
        rgbww ret;
        float limit = Model::instance().analogConfig(terminal).pwm_limit;
        auto input_type = Model::instance().analogConfig(terminal).input_type;
        auto output_type = Model::instance().analogConfig(terminal).output_type;

        auto scale8bit = [=] (uint16_t in) {
            return uint16_t(std::min(limit, float(in) * (1.0f / 255.0f)) * (PwmTimer::pwmPeriod));
        };

        auto scale16bit = [=] (uint16_t in) {
            return uint16_t(std::min(limit, float(in) * (1.0f / 65535.0f)) * (PwmTimer::pwmPeriod));
        };

        switch(input_type) {
        case INPUT_TYPE_dRGB8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale8bit(rgb.r);
                ret.g = scale8bit(rgb.g);
                ret.b = scale8bit(rgb.b);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW:
            case OUTPUT_TYPE_RGBW: {
                uint16_t r = scale8bit(rgb.r);
                uint16_t g = scale8bit(rgb.g);
                uint16_t b = scale8bit(rgb.b);
                uint16_t m = std::min(r, std::min(g, b));
                ret.r = r - m;
                ret.g = g - m;
                ret.b = b - m;
                ret.w = m;
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_dRGBW8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale8bit(rgb.r + rgb.w);
                ret.g = scale8bit(rgb.g + rgb.w);
                ret.b = scale8bit(rgb.b + rgb.w);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBW: {
                ret.r = scale8bit(rgb.r);
                ret.g = scale8bit(rgb.g);
                ret.b = scale8bit(rgb.b);
                ret.w = scale8bit(rgb.w);
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW: {
                ret.r = scale8bit(rgb.r);
                ret.g = scale8bit(rgb.g);
                ret.b = scale8bit(rgb.b);
                ret.w = scale8bit(rgb.w);
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_dRGBWWW8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale8bit(rgb.r + rgb.w + rgb.ww);
                ret.g = scale8bit(rgb.g + rgb.w + rgb.ww);
                ret.b = scale8bit(rgb.b + rgb.w + rgb.ww);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBW: {
                ret.r = scale8bit(rgb.r);
                ret.g = scale8bit(rgb.g);
                ret.b = scale8bit(rgb.b);
                ret.w = scale8bit(rgb.w + rgb.ww);
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW: {
                ret.r = scale8bit(rgb.r);
                ret.g = scale8bit(rgb.g);
                ret.b = scale8bit(rgb.b);
                ret.w = scale8bit(rgb.w);
                ret.ww = scale8bit(rgb.ww);
            } break;
            }
        } break;
        case INPUT_TYPE_sRGB8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                ret.r = uint16_t(rp);
                ret.g = uint16_t(gp);
                ret.b = uint16_t(bp);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW:
            case OUTPUT_TYPE_RGBW: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t mp = std::min(rp, std::min(gp, bp));
                ret.r = uint16_t(rp - mp);
                ret.g = uint16_t(gp - mp);
                ret.b = uint16_t(bp - mp);
                ret.w = uint16_t(mp);
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_sRGBW8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t wp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                ret.r = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), rp + wp));
                ret.g = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), gp + wp));
                ret.b = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), bp + wp));
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW:
            case OUTPUT_TYPE_RGBW: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t wp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                ret.r = uint16_t(rp);
                ret.g = uint16_t(gp);
                ret.b = uint16_t(bp);
                ret.w = uint16_t(wp);
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_sRGBWWW8: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t wp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                uint32_t wwp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                ret.r = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), rp + wp + wwp));
                ret.g = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), gp + wp + wwp));
                ret.b = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), bp + wp + wwp));
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBW: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t wp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                uint32_t wwp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].ww];
                ret.r = uint16_t(rp);
                ret.g = uint16_t(gp);
                ret.b = uint16_t(bp);
                ret.w = uint16_t(std::min(uint32_t(PwmTimer::pwmPeriod), wp + wwp));
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW: {
                uint32_t rp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].r];
                uint32_t gp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].g];
                uint32_t bp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].b];
                uint32_t wp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].w];
                uint32_t wwp = CIETransferfromsRGBTransferLookup::instance().lookup[_srgbww[terminal].ww];
                ret.r = uint16_t(rp);
                ret.g = uint16_t(gp);
                ret.b = uint16_t(bp);
                ret.w = uint16_t(wp);
                ret.ww = uint16_t(wwp);
            } break;
            }
        } break;
        case INPUT_TYPE_dRGB16: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale16bit(rgb.r);
                ret.g = scale16bit(rgb.g);
                ret.b = scale16bit(rgb.b);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW:
            case OUTPUT_TYPE_RGBW: {
                uint16_t r = scale16bit(rgb.r);
                uint16_t g = scale16bit(rgb.g);
                uint16_t b = scale16bit(rgb.b);
                uint16_t m = std::min(r, std::min(g, b));
                ret.r = r - m;
                ret.g = g - m;
                ret.b = b - m;
                ret.w = m;
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_dRGBW16: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale16bit(rgb.r + rgb.w);
                ret.g = scale16bit(rgb.g + rgb.w);
                ret.b = scale16bit(rgb.b + rgb.w);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBW: {
                ret.r = scale16bit(rgb.r);
                ret.g = scale16bit(rgb.g);
                ret.b = scale16bit(rgb.b);
                ret.w = scale16bit(rgb.w);
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW: {
                ret.r = scale16bit(rgb.r);
                ret.g = scale16bit(rgb.g);
                ret.b = scale16bit(rgb.b);
                ret.w = scale16bit(rgb.w);
                ret.ww = 0;
            } break;
            }
        } break;
        case INPUT_TYPE_dRGBWWW16: {
            switch(output_type) {
            case OUTPUT_TYPE_RGB: {
                ret.r = scale16bit(rgb.r + rgb.w + rgb.ww);
                ret.g = scale16bit(rgb.g + rgb.w + rgb.ww);
                ret.b = scale16bit(rgb.b + rgb.w + rgb.ww);
                ret.w = 0;
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBW: {
                ret.r = scale16bit(rgb.r);
                ret.g = scale16bit(rgb.g);
                ret.b = scale16bit(rgb.b);
                ret.w = scale16bit(rgb.w + rgb.ww);
                ret.ww = 0;
            } break;
            case OUTPUT_TYPE_RGBWWW: {
                ret.r = scale16bit(rgb.r);
                ret.g = scale16bit(rgb.g);
                ret.b = scale16bit(rgb.b);
                ret.w = scale16bit(rgb.w);
                ret.ww = scale16bit(rgb.ww);
            } break;
            }
        } break;
        }
        return ret;
    };

    switch(Model::instance().outputConfig()) {
    case Model::OUTPUT_CONFIG_DUAL_STRIP: {
    } break;
    case Model::OUTPUT_CONFIG_RGB_STRIP: {
        if (terminal == 0) {
            rgbww col = convert(_srgbww[terminal]);
            setPulse(0 + 0, col.r);
            setPulse(0 + 1, col.g);
            setPulse(0 + 2, col.b);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_DUAL_STRIP: {
        if (terminal == 0) {
            rgbww col = convert(_srgbww[terminal]);
            setPulse(3 + 0, col.r);
            setPulse(0 + 1, col.g);
            setPulse(0 + 2, col.b);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGBW_STRIP: {
        if (terminal == 0) {
            rgbww col = convert(_srgbww[terminal]);
            setPulse(0, col.r);
            setPulse(1, col.g);
            setPulse(2, col.b);
            setPulse(3, col.w);
        }
    } break;
    case Model::OUTPUT_CONFIG_RGB_RGB: {
        rgbww col = convert(_srgbww[terminal]);
        setPulse(terminal*3 + 0, col.r);
        setPulse(terminal*3 + 1, col.g);
        setPulse(terminal*3 + 2, col.b);
    } break;
    case Model::OUTPUT_CONFIG_RGBWWW: {
        if (terminal == 0) {
            rgbww col = convert(_srgbww[terminal]);
            setPulse(0, col.r);
            setPulse(1, col.g);
            setPulse(2, col.b);
            setPulse(3, col.w);
            setPulse(4, col.ww);
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

    PwmTimer0::instance().setPulse(0x0);
    PwmTimer1::instance().setPulse(0x0);
    PwmTimer2::instance().setPulse(0x0);
    PwmTimer3::instance().setPulse(0x0);
    PwmTimer5::instance().setPulse(0x0);
    PwmTimer6::instance().setPulse(0x0);
    
    DEBUG_PRINTF(("Driver up.\n"));
}

}
