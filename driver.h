/*
* driver.h
*
*  Created on: Sep 17, 2019
*      Author: Tinic Uro
*/

#ifndef LIGHTGUY_DRIVER_H_
#define LIGHTGUY_DRIVER_H_

#include <stdint.h>
#include <string.h>

namespace lightguy {

class rgb8 {
public:

    union {
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t x;
        };
        uint32_t rgbx;
    };

    rgb8() :
        r(0),
        g(0),
        b(0),
        x(0){
    }

    rgb8(const rgb8 &from) :
        r(from.r),
        g(from.g),
        b(from.b),
        x(0) {
    }

    explicit rgb8(uint8_t _r, uint8_t _g, uint8_t _b) :
        r(_r),
        g(_g),
        b(_b),
        x(0) {
    }
    
    uint32_t hex() const {
        return static_cast<uint32_t>((r<<16) | (g<<8) | b);
    }
    
    bool operator==(const rgb8 &c) const {
        return rgbx == c.rgbx;
    }

    bool operator!=(const rgb8 &c) const {
        return rgbx != c.rgbx;
    }
    
private:

    uint8_t sat8(const float v) const {
        return v < 0.0f ? uint8_t(0) : ( v > 1.0f ? uint8_t(0xFF) : uint8_t( v * 255.f ) );
    }

};

class Driver {
public:
    constexpr static size_t terminalN = 2;
    
    static Driver &instance();

    const rgb8 &rgb8CIE(size_t terminal) const { terminal %= terminalN; return _rgb8[terminal]; }
    void setRGB8CIE(size_t terminal, const rgb8 &rgb);

private:
    void maybeUpdateCIE();
    
    void setPulse(size_t idx, uint16_t pulse);

    bool initialized = false;
    void init();
    
    rgb8 _rgb8[terminalN];
    uint16_t cie_lookup[256];
    float pwm_limit = 0.0f;
};

};

#endif /* LIGHTGUY_DRIVER_H_ */
