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

    const rgbww &srgbwwCIE(size_t terminal) const { terminal %= terminalN; return _srgbww[terminal]; }
    void setsRGBWWCIE(size_t terminal, const rgbww &rgb);

	void sync(size_t terminal);
	
	void setRGBColorSpace(const RGBColorSpace &rgbSpace) { colorConverter.setRGBColorSpace(rgbSpace); }

private:
    void setPulse(size_t idx, uint16_t pulse);

    bool initialized = false;
    void init();
    
    rgbww _srgbww[terminalN];
    ColorSpaceConverter colorConverter; 
    CIETransferfromsRGBTransferLookup transferLookup;
};

};

#endif /* _DRIVER_H_ */
