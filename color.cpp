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
#include "color.h"
#include "pwmtimer.h"

#include "cmsis_gcc.h"

#include <algorithm>

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace lightkraken {
    

void CIETransferfromsRGBTransferLookup::init() {
    for (size_t c = 0; c<256; c++) {
        float f = float(c) * (1.0f / 255.0f);
        // from sRGB transfer to linear
        f = (f < 0.04045f) ? (f / 12.92f) : powf((f + 0.055f) / 1.055f, 2.4f);
        // linear to CIE transfer
        f = (f > 0.08f) ? powf( (f + 0.160f) / 1.160f, 3.0f) : (f / 9.03296296296296296294f);
        lookup[c] = uint16_t(f * float(PwmTimer::pwmPeriod));
    }
}

void ColorSpaceConverter::sRGB8toLEDPWM(
        uint8_t srgb_r, 
        uint8_t srgb_g, 
        uint8_t srgb_b,
        uint16_t pwm_l,
        uint16_t &pwm_r,
        uint16_t &pwm_g,
        uint16_t &pwm_b) const {
            
    float col[3];
    col[0] = float(srgb_r) * (1.0f / 255.0f);
    col[1] = float(srgb_g) * (1.0f / 255.0f);
    col[2] = float(srgb_b) * (1.0f / 255.0f);
    
    sRGB2sRGBL(col, col); // sRGB to Linear sRGB
    sRGBL2LEDL(col, col); // Linear sRGB -> XYZ -> Linear LED RGB
    LEDL2LED(col, col); // Linear LED RGB -> LED RGB
    
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
}

static constexpr int32_t fixed_shift = 24;
static constexpr int32_t fixed_post_shift = 8;

__attribute__((used))
static int32_t mul_fixed(int32_t x, int32_t y) {
    return int32_t((int64_t(x) * int64_t(y)) >> fixed_shift);
}


__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGB8toLEDRGB8(
    size_t len,
    const uint8_t *src,
    uint8_t *dst,
    uint8_t off_r,
    uint8_t off_g,
    uint8_t off_b,
    size_t in_channels,
    size_t out_channels) {

    for (size_t c = 0; c < len; c += in_channels) {

        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        int32_t lr = srgb_2_srgbl_lookup_fixed[src[0]];
        int32_t lg = srgb_2_srgbl_lookup_fixed[src[1]];
        int32_t lb = srgb_2_srgbl_lookup_fixed[src[2]];
        
        int32_t x = mul_fixed(srgbl2ledl_fixed[0], lr) + mul_fixed(srgbl2ledl_fixed[1], lg) + mul_fixed(srgbl2ledl_fixed[2], lb);
        int32_t y = mul_fixed(srgbl2ledl_fixed[3], lr) + mul_fixed(srgbl2ledl_fixed[4], lg) + mul_fixed(srgbl2ledl_fixed[5], lb);
        int32_t z = mul_fixed(srgbl2ledl_fixed[6], lr) + mul_fixed(srgbl2ledl_fixed[7], lg) + mul_fixed(srgbl2ledl_fixed[8], lb);

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        if ( y > theta ) {
            y = mul_fixed(y + delta, const_mul0);
            y = mul_fixed(y, mul_fixed(y, y)); 
        } else {
            y = mul_fixed(y, const_mul1);
        }

        if ( z > theta ) {
            z = mul_fixed(z + delta, const_mul0);
            z = mul_fixed(z, mul_fixed(z, z)); 
        } else {
            z = mul_fixed(z, const_mul1);
        }
        
        dst[off_r] = __USAT((x >> fixed_post_shift) >> 8, 7);
        dst[off_g] = __USAT((y >> fixed_post_shift) >> 8, 7);
        dst[off_b] = __USAT((z >> fixed_post_shift) >> 8, 7);

        src += in_channels;
        dst += out_channels;
    }
}

__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGBW8toLEDRGB8(
    size_t len,
    const uint8_t *src,
    uint8_t *dst,
    uint8_t off_r,
    uint8_t off_g,
    uint8_t off_b,
    size_t in_channels,
    size_t out_channels) {

    for (size_t c = 0; c < len; c += in_channels) {

        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        uint32_t r = src[0];
        uint32_t g = src[1];
        uint32_t b = src[2];
        uint32_t w = src[3];

        int32_t lr = srgb_2_srgbl_lookup_fixed[std::clamp(r+w, uint32_t(0), uint32_t(255))];
        int32_t lg = srgb_2_srgbl_lookup_fixed[std::clamp(g+w, uint32_t(0), uint32_t(255))];
        int32_t lb = srgb_2_srgbl_lookup_fixed[std::clamp(b+w, uint32_t(0), uint32_t(255))];
        
        int32_t x = mul_fixed(srgbl2ledl_fixed[0], lr) + mul_fixed(srgbl2ledl_fixed[1], lg) + mul_fixed(srgbl2ledl_fixed[2], lb);
        int32_t y = mul_fixed(srgbl2ledl_fixed[3], lr) + mul_fixed(srgbl2ledl_fixed[4], lg) + mul_fixed(srgbl2ledl_fixed[5], lb);
        int32_t z = mul_fixed(srgbl2ledl_fixed[6], lr) + mul_fixed(srgbl2ledl_fixed[7], lg) + mul_fixed(srgbl2ledl_fixed[8], lb);

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        if ( y > theta ) {
            y = mul_fixed(y + delta, const_mul0);
            y = mul_fixed(y, mul_fixed(y, y)); 
        } else {
            y = mul_fixed(y, const_mul1);
        }

        if ( z > theta ) {
            z = mul_fixed(z + delta, const_mul0);
            z = mul_fixed(z, mul_fixed(z, z)); 
        } else {
            z = mul_fixed(z, const_mul1);
        }
        
        dst[off_r] = __USAT((x >> fixed_post_shift) >> 8, 7);
        dst[off_g] = __USAT((y >> fixed_post_shift) >> 8, 7);
        dst[off_b] = __USAT((z >> fixed_post_shift) >> 8, 7);

        src += in_channels;
        dst += out_channels;
    }
}

__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGB8toLEDRGBW8(
    size_t len,
    const uint8_t *src,
    uint8_t *dst,
    uint8_t off_r,
    uint8_t off_g,
    uint8_t off_b,
    uint8_t off_w,
    size_t in_channels,
    size_t out_channels) {

    for (size_t c = 0; c < len; c += in_channels) {

        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        int32_t r = src[0];
        int32_t g = src[1];
        int32_t b = src[2];
		int32_t w = std::min(r, std::min(g, b));

        int32_t lr = srgb_2_srgbl_lookup_fixed[r - w];
        int32_t lg = srgb_2_srgbl_lookup_fixed[g - w];
        int32_t lb = srgb_2_srgbl_lookup_fixed[b - w];
        int32_t lw = srgb_2_srgbl_lookup_fixed[    w];
        
        int32_t x = mul_fixed(srgbl2ledl_fixed[0], lr) + mul_fixed(srgbl2ledl_fixed[1], lg) + mul_fixed(srgbl2ledl_fixed[2], lb);
        int32_t y = mul_fixed(srgbl2ledl_fixed[3], lr) + mul_fixed(srgbl2ledl_fixed[4], lg) + mul_fixed(srgbl2ledl_fixed[5], lb);
        int32_t z = mul_fixed(srgbl2ledl_fixed[6], lr) + mul_fixed(srgbl2ledl_fixed[7], lg) + mul_fixed(srgbl2ledl_fixed[8], lb);
        w = lw;

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        if ( y > theta ) {
            y = mul_fixed(y + delta, const_mul0);
            y = mul_fixed(y, mul_fixed(y, y)); 
        } else {
            y = mul_fixed(y, const_mul1);
        }

        if ( z > theta ) {
            z = mul_fixed(z + delta, const_mul0);
            z = mul_fixed(z, mul_fixed(z, z)); 
        } else {
            z = mul_fixed(z, const_mul1);
        }

        if ( w > theta ) {
            w = mul_fixed(w + delta, const_mul0);
            w = mul_fixed(w, mul_fixed(w, w)); 
        } else {
            w = mul_fixed(w, const_mul1);
        }

        dst[off_r] = __USAT((x >> fixed_post_shift) >> 8, 7);
        dst[off_g] = __USAT((y >> fixed_post_shift) >> 8, 7);
        dst[off_b] = __USAT((z >> fixed_post_shift) >> 8, 7);
        dst[off_w] = __USAT((w >> fixed_post_shift) >> 8, 7);

        src += in_channels;
        dst += out_channels;
    }
}

__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGB8TransfertoLED8Transfer(
    size_t len,
    const uint8_t *src,
    uint8_t *dst,
    uint8_t off_in,
    uint8_t off_out,
    size_t channels) {

    for (size_t c = 0; c < len; c += channels) {
        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        int32_t x = srgb_2_srgbl_lookup_fixed[src[off_in]];

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        dst[off_out] = __USAT((x >> fixed_post_shift) >> 8, 7);
        
        src += channels;
        dst += channels;
    }
}

__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGB8toLED16(
    size_t len,
    const uint8_t *src,
    uint32_t *dst,
    uint8_t off_r,
    uint8_t off_g,
    uint8_t off_b,
    size_t channels) {

    for (size_t c = 0; c < len; c += channels) {

        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        int32_t lr = srgb_2_srgbl_lookup_fixed[src[0]];
        int32_t lg = srgb_2_srgbl_lookup_fixed[src[1]];
        int32_t lb = srgb_2_srgbl_lookup_fixed[src[2]];
        
        int32_t x = mul_fixed(srgbl2ledl_fixed[0], lr) + mul_fixed(srgbl2ledl_fixed[1], lg) + mul_fixed(srgbl2ledl_fixed[2], lb);
        int32_t y = mul_fixed(srgbl2ledl_fixed[3], lr) + mul_fixed(srgbl2ledl_fixed[4], lg) + mul_fixed(srgbl2ledl_fixed[5], lb);
        int32_t z = mul_fixed(srgbl2ledl_fixed[6], lr) + mul_fixed(srgbl2ledl_fixed[7], lg) + mul_fixed(srgbl2ledl_fixed[8], lb);

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        if ( y > theta ) {
            y = mul_fixed(y + delta, const_mul0);
            y = mul_fixed(y, mul_fixed(y, y)); 
        } else {
            y = mul_fixed(y, const_mul1);
        }

        if ( z > theta ) {
            z = mul_fixed(z + delta, const_mul0);
            z = mul_fixed(z, mul_fixed(z, z)); 
        } else {
            z = mul_fixed(z, const_mul1);
        }
        
        dst[off_r] = __USAT(x >> fixed_post_shift, 15);
        dst[off_g] = __USAT(y >> fixed_post_shift, 15);
        dst[off_b] = __USAT(z >> fixed_post_shift, 15);

        src += channels;
        dst += channels;
    }
}

__attribute__ ((hot, optimize("O3")))
void ColorSpaceConverter::sRGB8TransfertoLED16Transfer(
    size_t len,
    const uint8_t *src,
    uint32_t *dst,
    uint8_t off_in,
    uint8_t off_out,
    size_t channels) {

    for (size_t c = 0; c < len; c += channels) {
        constexpr const int32_t theta = int32_t(0.080f * float(1UL<<fixed_shift));
        constexpr const int32_t delta = int32_t(0.160f * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul0 = int32_t((1.0f / 1.160f) * float(1UL<<fixed_shift));
        constexpr const int32_t const_mul1 = int32_t((1.0f / 9.03296296296296296294f) * float(1UL<<fixed_shift));

        int32_t x = srgb_2_srgbl_lookup_fixed[src[off_in]];

        if ( x > theta ) {
            x = mul_fixed(x + delta, const_mul0);
            x = mul_fixed(x, mul_fixed(x, x)); 
        } else {
            x = mul_fixed(x, const_mul1);
        }

        dst[off_out] = __USAT(x >> fixed_post_shift, 15);
        
        src += channels;
        dst += channels;
    }
}

void ColorSpaceConverter::setRGBColorSpace(const RGBColorSpace &rgbSpace) {
    // Generate XYZ to LED matrices
    generateRGBMatrix(
        rgbSpace.xw, rgbSpace.yw,
        rgbSpace.xr, rgbSpace.yr,
        rgbSpace.xg, rgbSpace.yg,
        rgbSpace.xb, rgbSpace.yb,
        ledl2srgbl, // ledl2xyz really here
        srgbl2ledl); // xyz2ledl really here
        
    // Concat sRGB conversions
    static float xyz2srgbl[9] = {
        3.2404542f, -1.5371385f, -0.4985314f,
        -0.9692660f,  1.8760108f,  0.0415560f,
        0.0556434f, -0.2040259f,  1.0572252f,
    };

    concatMatrix(ledl2srgbl, ledl2srgbl, xyz2srgbl);

    static float srgbl2xyz[9] = {
        0.4124564f,  0.3575761f,  0.1804375f,
        0.2126729f,  0.7151522f,  0.0721750f,
        0.0193339f,  0.1191920f,  0.9503041f,
    };

    concatMatrix(srgbl2ledl, srgbl2xyz, srgbl2ledl);

    for (size_t c = 0; c < 256; c++) {
        float v = float(c) * (1.0f / 255.0f);
        srgb_2_srgbl_lookup_fixed[c] = int32_t((v < 0.04045f) ? (v / 12.92f) : powf((v + 0.055f) / 1.055f, 2.4f) * float(1UL<<fixed_shift));
    }

    for (size_t c = 0; c < 9; c++) {
        srgbl2ledl_fixed[c] = int32_t(srgbl2ledl[c] * float(1UL<<fixed_shift));
        ledl2srgbl_fixed[c] = int32_t(ledl2srgbl[c] * float(1UL<<fixed_shift));
    }

}

void ColorSpaceConverter::sRGBL2LEDL(float *ledl, const float *srgbl) const {
    float r = srgbl2ledl[0]*srgbl[0] + srgbl2ledl[1]*srgbl[1] + srgbl2ledl[2]*srgbl[2];
    float g = srgbl2ledl[3]*srgbl[0] + srgbl2ledl[4]*srgbl[1] + srgbl2ledl[5]*srgbl[2];
    float b = srgbl2ledl[6]*srgbl[0] + srgbl2ledl[7]*srgbl[1] + srgbl2ledl[8]*srgbl[2];
    ledl[0] = r;
    ledl[1] = g;
    ledl[2] = b;
}

void ColorSpaceConverter::LEDL2sRGBL(float *ledl, const float *srgbl) const {
    float r = ledl2srgbl[0]*srgbl[0] + ledl2srgbl[1]*srgbl[1] + ledl2srgbl[2]*srgbl[2];
    float g = ledl2srgbl[3]*srgbl[0] + ledl2srgbl[4]*srgbl[1] + ledl2srgbl[5]*srgbl[2];
    float b = ledl2srgbl[6]*srgbl[0] + ledl2srgbl[7]*srgbl[1] + ledl2srgbl[8]*srgbl[2];
    ledl[0] = r;
    ledl[1] = g;
    ledl[2] = b;
}

void ColorSpaceConverter::sRGB2sRGBL(float *srgbl, const float *srgb) const {
    float r = (srgb[0] < 0.04045f) ? (srgb[0] / 12.92f) : powf((srgb[0] + 0.055f) / 1.055f, 2.4f);
    float g = (srgb[1] < 0.04045f) ? (srgb[1] / 12.92f) : powf((srgb[1] + 0.055f) / 1.055f, 2.4f);
    float b = (srgb[2] < 0.04045f) ? (srgb[2] / 12.92f) : powf((srgb[2] + 0.055f) / 1.055f, 2.4f);
    srgbl[0] = r;
    srgbl[1] = g;
    srgbl[2] = b;
}

void ColorSpaceConverter::sRGBL2sRGB(float *srgb, const float *srgbl) const {
    float r = (srgbl[0] < 0.0031308f) ? (srgbl[0] * 12.92f) : ((1.055f * powf(srgbl[0] , 1.0f / 2.4f)) - 0.055f);
    float g = (srgbl[1] < 0.0031308f) ? (srgbl[1] * 12.92f) : ((1.055f * powf(srgbl[1] , 1.0f / 2.4f)) - 0.055f);
    float b = (srgbl[2] < 0.0031308f) ? (srgbl[2] * 12.92f) : ((1.055f * powf(srgbl[2] , 1.0f / 2.4f)) - 0.055f);
    srgb[0] = r;
    srgb[1] = g;
    srgb[2] = b;
}

void ColorSpaceConverter::LEDL2LED(float *led, const float *ledl) const {
    float r = (ledl[0] > 0.08f) ? powf( (ledl[0] + 0.160f) / 1.160f, 3.0f) : (ledl[0] / 9.03296296296296296294f);
    float g = (ledl[1] > 0.08f) ? powf( (ledl[1] + 0.160f) / 1.160f, 3.0f) : (ledl[1] / 9.03296296296296296294f);
    float b = (ledl[2] > 0.08f) ? powf( (ledl[2] + 0.160f) / 1.160f, 3.0f) : (ledl[2] / 9.03296296296296296294f);
    led[0] = r;
    led[1] = g;
    led[2] = b;
}

void ColorSpaceConverter::LED2LEDL(float *ledl, const float *led) const {
    float r = (led[0] > 0.00885645167903563081f) ? (( powf(led[0], 1.0f / 3.0f) * 1.160f) - 0.160f) : (led[0] * 9.03296296296296296294f);
    float g = (led[1] > 0.00885645167903563081f) ? (( powf(led[1], 1.0f / 3.0f) * 1.160f) - 0.160f) : (led[1] * 9.03296296296296296294f);
    float b = (led[2] > 0.00885645167903563081f) ? (( powf(led[2], 1.0f / 3.0f) * 1.160f) - 0.160f) : (led[2] * 9.03296296296296296294f);
    ledl[0] = r;
    ledl[1] = g;
    ledl[2] = b;
}

void ColorSpaceConverter::invertMatrix(float *r, const float *a) const {
    float d = 1.0f/(+a[0]*(a[4]*a[8]-a[7]*a[5])
                    -a[1]*(a[3]*a[8]-a[5]*a[6])
                    +a[2]*(a[3]*a[7]-a[4]*a[6]));
    float i[9];
    i[0] = (a[4]*a[8] - a[7]*a[5]) * d;
    i[1] = (a[2]*a[7] - a[1]*a[8]) * d;
    i[2] = (a[1]*a[5] - a[2]*a[4]) * d;
    i[3] = (a[5]*a[6] - a[3]*a[8]) * d;
    i[4] = (a[0]*a[8] - a[2]*a[6]) * d;
    i[5] = (a[3]*a[2] - a[0]*a[5]) * d;
    i[6] = (a[3]*a[7] - a[6]*a[4]) * d;
    i[7] = (a[6]*a[1] - a[0]*a[7]) * d;
    i[8] = (a[0]*a[4] - a[3]*a[1]) * d; 
    memcpy(r, i, sizeof(i));
}

void ColorSpaceConverter::concatMatrix(float *d, const float *a, const float *b) const {
    float m[9];
    for (size_t i=0; i<3; i++ ) {
        for (size_t j=0; j<3; j++ ) {
            m[i*3+j] = (a[i*3+0] * b[0*3+j]) +
                       (a[i*3+1] * b[1*3+j]) +
                       (a[i*3+2] * b[2*3+j]);
        }
    }
    memcpy(d, m, sizeof(m));
}

void ColorSpaceConverter::generateRGBMatrix(
        float xw, float yw,
        float xr, float yr, 
        float xg, float yg, 
        float xb, float yb,
        float *rgb2xyz,
        float *xyz2rgb) const {

    float Xw = xw / yw;
    float Yw = 1.0f;
    float Zw = ( 1.0f - xw - yw) / yw;

    float Xr = xr / yr;
    float Yr = 1.0f;
    float Zr = ( 1.0f - xr - yr) / yr;

    float Xg = xg / yg;
    float Yg = 1.0;
    float Zg = ( 1.0f - xg - yg) / yg;

    float Xb = xb / yb;
    float Yb = 1.0f;
    float Zb = ( 1.0f - xb - yb) / yb;

    float matrix[9];
    matrix[0] = Xr; matrix[1] = Xg; matrix[2] = Xb;
    matrix[3] = Yr; matrix[4] = Yg; matrix[5] = Yb;
    matrix[6] = Zr; matrix[7] = Zg; matrix[8] = Zb;

    invertMatrix(matrix, matrix);

    float Sr = matrix[0]*Xw + matrix[1]*Yw + matrix[2]*Zw;
    float Sg = matrix[3]*Xw + matrix[4]*Yw + matrix[5]*Zw;
    float Sb = matrix[6]*Xw + matrix[7]*Yw + matrix[8]*Zw;

    rgb2xyz[0] = Sr*Xr; rgb2xyz[1] = Sg*Xg; rgb2xyz[2] = Sb*Xb;
    rgb2xyz[3] = Sr*Yr; rgb2xyz[4] = Sg*Yg; rgb2xyz[5] = Sb*Yb;
    rgb2xyz[6] = Sr*Zr; rgb2xyz[7] = Sg*Zg; rgb2xyz[8] = Sb*Zb;

    invertMatrix(xyz2rgb, rgb2xyz);
}

}

