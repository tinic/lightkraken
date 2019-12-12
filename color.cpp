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

void ColorSpaceConverter::CIE2CIEL(float *ciel, const float *cie) const {
    float r = (cie[0] > 0.08f) ? powf( (cie[0] + 0.160f) / 1.160f, 3.0f) : (cie[0] / 9.03296296296296296294f);
    float g = (cie[1] > 0.08f) ? powf( (cie[1] + 0.160f) / 1.160f, 3.0f) : (cie[1] / 9.03296296296296296294f);
    float b = (cie[2] > 0.08f) ? powf( (cie[2] + 0.160f) / 1.160f, 3.0f) : (cie[2] / 9.03296296296296296294f);
    ciel[0] = r;
    ciel[1] = g;
    ciel[2] = b;
}

void ColorSpaceConverter::CIEL2CIE(float *cie, const float *ciel) const {
    float r = (ciel[0] > 0.00885645167903563081f) ? (( powf(ciel[0], 1.0f / 3.0f) * 1.160f) - 0.160f) : (ciel[0] * 9.03296296296296296294f);
    float g = (ciel[1] > 0.00885645167903563081f) ? (( powf(ciel[1], 1.0f / 3.0f) * 1.160f) - 0.160f) : (ciel[1] * 9.03296296296296296294f);
    float b = (ciel[2] > 0.00885645167903563081f) ? (( powf(ciel[2], 1.0f / 3.0f) * 1.160f) - 0.160f) : (ciel[2] * 9.03296296296296296294f);
    cie[0] = r;
    cie[1] = g;
    cie[2] = b;
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

rgb::rgb(const hsv &from) {
	float v = from.v;
	float h = from.h;
	float s = from.s;

	int32_t rd = static_cast<int32_t>( 6.0f * h );
	float f = h * 6.0f - rd;
	float p = v * (1.0f - s);
	float q = v * (1.0f - f * s);
	float t = v * (1.0f - (1.0f - f) * s);

	switch ( rd  % 6 ) {
		default:
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q; break;
	}
}

rgb8::rgb8(const rgb &from) {
	r = sat8(from.r);
	g = sat8(from.g);
	b = sat8(from.b);
	x = 0;
}

}

