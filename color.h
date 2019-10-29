#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>

namespace lightkraken {

class rgbww {
public:

    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t w;
    uint16_t ww;

    rgbww() :
        r(0),
        g(0),
        b(0),
        w(0),
        ww(0) {
    }

    rgbww(const rgbww &from) :
        r(from.r),
        g(from.g),
        b(from.b),
        w(from.w),
        ww(from.ww) {
    }

    explicit rgbww(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _w = 0, uint8_t _ww = 0) :
        r(_r),
        g(_g),
        b(_b),
        w(_w), 
        ww(_ww) {
    }
    
private:

};

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
        x(from.x) {
    }

    explicit rgb8(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _x = 0) :
        r(_r),
        g(_g),
        b(_b),
        x(_x) {
    }
    
    bool operator==(const rgb8 &c) const {
        return rgbx == c.rgbx;
    }

    bool operator!=(const rgb8 &c) const {
        return rgbx != c.rgbx;
    }
    
private:

};

}

#endif  // #ifndef _COLOR_H_
