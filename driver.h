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
#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdint.h>
#include <string.h>

#include "./color.h"

namespace lightkraken {

class Driver {
public:

    enum OutputTypes {
        OUTPUT_TYPE_RGB,
        OUTPUT_TYPE_RGBW,
        OUTPUT_TYPE_RGBWWW
    };

    enum InputTypes {
        INPUT_TYPE_dRGB,
        INPUT_TYPE_dRGBW,
        INPUT_TYPE_dRGBWWW,
        INPUT_TYPE_sRGB,
        INPUT_TYPE_sRGBW,
        INPUT_TYPE_sRGBWWW,
    };

    constexpr static size_t terminalN = 2;
    
    static Driver &instance();

    const rgbww &srgbwwCIE(size_t terminal) const { terminal %= terminalN; return _srgbww[terminal]; }
    void setsRGBWWCIE(size_t terminal, const rgbww &rgb);

    void sync(size_t terminal);
    
    void setRGBColorSpace(size_t terminal, const RGBColorSpace &rgbSpace) { colorConverter[terminal].setRGBColorSpace(rgbSpace); }
    void setPWMLimit(size_t terminal, float value) { pwm_limit[terminal] = value; }

private:
    void setPulse(size_t idx, uint16_t pulse);

    bool initialized = false;
    void init();
    
    rgbww _srgbww[terminalN];
    float pwm_limit[terminalN];
    ColorSpaceConverter colorConverter[terminalN]; 
    CIETransferfromsRGBTransferLookup transferLookup;
};

};

#endif /* _DRIVER_H_ */
