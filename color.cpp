
#include "color.h"

#include <math.h>
#include <memory.h>

namespace lightkraken {

void ColorSpaceConverter::sRGBtoLEDPWM(
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
	sRGBtoLED(col);
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

void ColorSpaceConverter::setRGBSpace(
	float xw, float yw,
	float xr, float yr,
	float xg, float yg, 
	float xb, float yb) {
	
    // Generate XYZ to LED matrices
    generateRGBMatrix(
        xw, yw,
        xr, yr,
        xg, yg,
        xb, yb,
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
}

void ColorSpaceConverter::sRGBtoLED(float *col) const {
	sRGB2sRGBL(col, col); // sRGB to Linear sRGB
	sRGBL2LEDL(col, col); // sLinear sRGB -> XYZ -> Linear LED RGB
#ifdef TEST_SRGB_IDENTITY
    sRGBL2sRGB(col, col); // Linera LED RGB -> LED RGB
#else  // #ifdef TEST_SRGB_IDENTITY
    LEDL2LED(col, col); // Linera LED RGB -> LED RGB
#endif  // #ifdef TEST_SRGB_IDENTITY
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

float ColorSpaceConverter::srgbl2ledl[9];
float ColorSpaceConverter::ledl2srgbl[9];

}

