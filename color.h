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
#ifndef _COLOR_H_
#define _COLOR_H_

#include <stdint.h>
#include <string.h>

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

    void sRGB8toLED8(
            size_t len,
            const uint8_t *src,
            uint8_t *dst,
            uint8_t off_r,
            uint8_t off_g,
            uint8_t off_b,
            size_t channels);

    void sRGB8TransfertoLED8Transfer(
            size_t len,
            const uint8_t *src,
            uint8_t *dst,
            uint8_t off_in,
            uint8_t off_out,
            size_t channels);

    void sRGB8toLED16(
            size_t len,
            const uint8_t *src,
            uint32_t *dst,
            uint8_t off_r,
            uint8_t off_g,
            uint8_t off_b,
            size_t channels);

    void sRGB8TransfertoLED16Transfer(
            size_t len,
            const uint8_t *src,
            uint32_t *dst,
            uint8_t off_in,
            uint8_t off_out,
            size_t channels);
    
    void setRGBColorSpace(const RGBColorSpace &rgbSpace);

private:

    float srgbl2ledl[9];
    float ledl2srgbl[9];
    
    int32_t srgb_2_srgbl_lookup_fixed[256];
    int32_t srgbl2ledl_fixed[9];
    int32_t ledl2srgbl_fixed[9];

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
