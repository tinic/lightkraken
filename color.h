#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>

namespace lightkraken {

//#define TEST_SRGB_IDENTITY

struct RGBColorSpace {

	void setsRGB() {
        xw = 0.31271f; yw = 0.32902f; // D65 white point
        xr = 0.64000f; yr = 0.33000f;
        xg = 0.30000f; yg = 0.60000f;
        xb = 0.15000f; yb = 0.06000f;
	}

	void setLED() {
        xw = 0.34567f; yw = 0.35850f; // D50 white point
        xr = 0.67630f; yr = 0.32370f; // from http://ww1.microchip.com/downloads/en/AppNotes/00001562B.pdf
        xg = 0.20880f; yg = 0.74070f;
        xb = 0.14050f; yb = 0.03910f;
	}

	float xw; float yw;
	float xr; float yr;
	float xg; float yg;
	float xb; float yb;
};

class CIETransferfromsRGBTransferLookup {
public:
	void init();
    uint16_t lookup[256];
};

class ColorSpaceConverter {
public:

    void sRGB8toLEDPWM(
            uint8_t srgb_r,
            uint8_t srgb_g,
            uint8_t srgb_b,
            uint16_t pwm_l,
            uint16_t &pwm_r,
            uint16_t &pwm_g,
            uint16_t &pwm_b) const;

    void setRGBColorSpace(const RGBColorSpace &rgbSpace);

private:

    static float srgbl2ledl[9];
    static float ledl2srgbl[9];

    void sRGBtoLED(float *col) const;
    void sRGBL2LEDL(float *ledl, const float *srgbl) const;
    void LEDL2sRGBL(float *ledl, const float *srgbl) const;
    void sRGB2sRGBL(float *srgbl, const float *srgb) const;
    void sRGBL2sRGB(float *srgb, const float *srgbl) const;
    void LEDL2LED(float *led, const float *ledl) const;
    void LED2LEDL(float *ledl, const float *led) const;
    void invertMatrix(float *r, const float *a) const;
    void concatMatrix(float *d, const float *a, const float *b) const;
    void generateRGBMatrix(    float xw, float yw,
                            float xr, float yr,
                            float xg, float yg,
                            float xb, float yb,
                            float *rgb2xyz,
                            float *xyz2rgb) const;
};


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
