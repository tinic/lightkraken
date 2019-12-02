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
#include <algorithm>

namespace lightkraken {



//#define TEST_SRGB_IDENTITY

struct RGBColorSpace {

    void setsRGB() {
        xw = 0.31271f; yw = 0.32902f; // D65 white point
        xr = 0.64000f; yr = 0.33000f;
        xg = 0.30000f; yg = 0.60000f;
        xb = 0.15000f; yb = 0.06000f;
    }

    void setWorldSemiLED() {
        xw = 0.31271f; yw = 0.32902f; // D65 white point
        xr = 0.68930f; yr = 0.30740f; // from world-semi.com
        xg = 0.13940f; yg = 0.72140f;
        xb = 0.13180f; yb = 0.07200f;
    }

    void setCreeLED() {
        xw = 0.35000f; yw = 0.33000f; // D65 white point
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

	static constexpr int32_t fixed_shift = 24;
	static constexpr int32_t fixed_post_shift = 8;
	static constexpr int32_t fixed_post_clamp = 1UL << (fixed_shift - fixed_post_shift);

    void setRGBColorSpace(const RGBColorSpace &rgbSpace);

	inline void sRGB8toLEDPWM(
			uint8_t srgb_r, 
			uint8_t srgb_g, 
			uint8_t srgb_b,
			uint16_t pwm_l,
			uint16_t &pwm_r,
			uint16_t &pwm_g,
			uint16_t &pwm_b) const {
#if 1
		int32_t lr = srgb_2_srgbl_lookup_fixed[srgb_r];
		int32_t lg = srgb_2_srgbl_lookup_fixed[srgb_g];
		int32_t lb = srgb_2_srgbl_lookup_fixed[srgb_b];
	
		int32_t x = mul_fixed(srgbl2ledl_fixed[0], lr) + mul_fixed(srgbl2ledl_fixed[1], lg) + mul_fixed(srgbl2ledl_fixed[2], lb);
		int32_t y = mul_fixed(srgbl2ledl_fixed[3], lr) + mul_fixed(srgbl2ledl_fixed[4], lg) + mul_fixed(srgbl2ledl_fixed[5], lb);
		int32_t z = mul_fixed(srgbl2ledl_fixed[6], lr) + mul_fixed(srgbl2ledl_fixed[7], lg) + mul_fixed(srgbl2ledl_fixed[8], lb);

		pwm_r = uint16_t((std::clamp((x >> fixed_post_shift), int32_t(0), fixed_post_clamp) * pwm_l) / fixed_post_clamp);
		pwm_g = uint16_t((std::clamp((y >> fixed_post_shift), int32_t(0), fixed_post_clamp) * pwm_l) / fixed_post_clamp);
		pwm_b = uint16_t((std::clamp((z >> fixed_post_shift), int32_t(0), fixed_post_clamp) * pwm_l) / fixed_post_clamp);
#else            
		float col[3];
		col[0] = float(srgb_r) * (1.0f / 255.0f);
		col[1] = float(srgb_g) * (1.0f / 255.0f);
		col[2] = float(srgb_b) * (1.0f / 255.0f);
	
		sRGB2sRGBL(col, col); // sRGB to Linear sRGB
		sRGBL2LEDL(col, col); // Linear sRGB -> XYZ -> Linear LED RGB
	
		for (size_t c = 0; c < 3; c++) {
			if (col[c] < 0.0f) {
				col[c] = 0.0f;
			}
			if (col[c] > 1.0f) {
				col[c] = 1.0f;
			}
		}
	
		pwm_r = uint16_t(col[0]*float(pwm_l));
		pwm_g = uint16_t(col[1]*float(pwm_l));
		pwm_b = uint16_t(col[2]*float(pwm_l));
#endif            
	}

private:

	inline int32_t mul_fixed(int32_t x, int32_t y) const {
    	return int32_t((int64_t(x) * int64_t(y)) >> fixed_shift);
	}

    float srgbl2ledl[9];
    float ledl2srgbl[9];
    
    int32_t srgb_2_srgbl_lookup_fixed[256];
    int32_t srgbl2ledl_fixed[9];
    int32_t ledl2srgbl_fixed[9];

    void sRGBL2LEDL(float *ledl, const float *srgbl) const;
    void LEDL2sRGBL(float *ledl, const float *srgbl) const;
    void sRGB2sRGBL(float *srgbl, const float *srgb) const;
    void sRGBL2sRGB(float *srgb, const float *srgbl) const;
    void CIE2CIEL(float *cie, const float *ciel) const;
    void CIEL2CIE(float *ciel, const float *cie) const;
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
