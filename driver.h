/*
* driver.h
*
*  Created on: Sep 17, 2019
*      Author: Tinic Uro
*/

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdint.h>
#include <string.h>

#include "./color.h"

namespace lightkraken {

class Driver {
public:
    constexpr static size_t terminalN = 2;
    
    static Driver &instance();

    const rgbww &rgbwwCIE(size_t terminal) const { terminal %= terminalN; return _rgbww[terminal]; }
    void setRGBWWCIE(size_t terminal, const rgbww &rgb);

private:
    void maybeUpdateCIE();
    
    void setPulse(size_t idx, uint16_t pulse);

    bool initialized = false;
    void init();
    
    rgbww _rgbww[terminalN];
    uint16_t cie_lookup[256];
    float pwm_limit = 0.0f;
};

};

#endif /* _DRIVER_H_ */
