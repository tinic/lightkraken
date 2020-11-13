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

#if __cplusplus < 201703
#ifndef _LEGACY_CLAMP_
#define _LEGACY_CLAMP_
namespace std {
template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}
}
#endif  // #ifndef _LEGACY_CLAMP_
#endif  // #if __cplusplus < 201703

namespace lightkraken {

//#define TEST_SRGB_IDENTITY

struct RGBColorSpace {

    RGBColorSpace() {
        setsRGB();
    }

    void setsRGB() {
        xw = 0.31271f; yw = 0.32902f; // D65 white point
        xr = 0.64000f; yr = 0.33000f;
        xg = 0.30000f; yg = 0.60000f;
        xb = 0.15000f; yb = 0.06000f;
    }

    float xw; float yw;
    float xr; float yr;
    float xg; float yg;
    float xb; float yb;
};

class CIETransferfromsRGBTransferLookup {
public:
    static CIETransferfromsRGBTransferLookup &instance();

    uint16_t lookup[256];

private:
    void init();
	bool initialized = false;
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

class rgb;

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

    explicit rgb8(const rgb &from);
    
    uint8_t red() const  { return r; }
    uint8_t green() const  { return g; }
    uint8_t blue() const  { return b; }
    
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

class hsv;

class rgb {
public:
	float r;
	float g;
	float b;

	rgb() :
		r(0.0f),
		g(0.0f),
		b(0.0f){
	}

	rgb(const rgb &from) :
		r(from.r),
		g(from.g),
		b(from.b) {
	}

	explicit rgb(const uint32_t color) {
        r = static_cast<float>((color>>16)&0xFF) * (1.0f/255.0f);
        g = static_cast<float>((color>> 8)&0xFF) * (1.0f/255.0f);
        b = static_cast<float>((color>> 0)&0xFF) * (1.0f/255.0f);
	}
	
	explicit rgb(const rgb8 &from) {
        r = static_cast<float>(from.r) * (1.0f/255.0f);
        g = static_cast<float>(from.g) * (1.0f/255.0f);
        b = static_cast<float>(from.b) * (1.0f/255.0f);
    }

	explicit rgb(const hsv &from);

	rgb(float _r, float _g, float _b) :
		r(_r),
		g(_g),
		b(_b) {
	}

	void set(float _r, float _g, float _b) {
		r = _r;
		g = _g;
		b = _b;
	}

	rgb &operator+=(const rgb &v) {
		r += v.r;
		g += v.g;
		b += v.b;
		return *this;
	}

	friend rgb operator+(rgb a, const rgb &_b) {
		a += _b;
		return a;
	}

	rgb &operator-=(const rgb &v) {
		r -= v.r;
		g -= v.g;
		b -= v.b;
		return *this;
	}

	friend rgb operator-(rgb a, const rgb &_b) {
		a -= _b;
		return a;
	}

	rgb &operator*=(const rgb &v) {
		r *= v.r;
		g *= v.g;
		b *= v.b;
		return *this;
	}

	friend rgb operator*(rgb a, const rgb &_b) {
		a *= _b;
		return a;
	}

	rgb &operator*=(float v) {
		r *= v;
		g *= v;
		b *= v;
		return *this;
	}

	friend rgb operator*(rgb a, float v) {
		a *= v;
		return a;
	}

	rgb &operator/=(const rgb &v) {
		r /= v.r;
		g /= v.g;
		b /= v.b;
		return *this;
	}

	friend rgb operator/(rgb a, const rgb &_b) {
		a /= _b;
		return a;
	}

	rgb &operator/=(float v) {
		r /= v;
		g /= v;
		b /= v;
		return *this;
	}

	friend rgb operator/(rgb a, float v) {
		a /= v;
		return a;
	}

private:
};

class hsv {
public:
	float h;
	float s;
	float v;

	hsv() :
		h(0.0f),
		s(0.0f),
		v(0.0f) {
	}

	hsv(float _h, float _s, float _v) :
		h(_h),
		s(_s),
		v(_v) {
	}

	hsv(const hsv &from) :
		h(from.h),
		s(from.s),
		v(from.v) {
	}

	explicit hsv(const rgb &from) {
		float hi = std::max(std::max(from.r, from.g), from.b);
		float lo = std::min(std::max(from.r, from.g), from.b);
		float d = hi - lo;

		h = 0.0f;
		s = 0.0f;
		v = hi;

		if ( ( v > 0.00001f ) &&
			 ( d > 0.00001f ) ) {
			s = d / v;
			if( hi == from.r ) {
				h = (60.0f/360.0f) * (from.g - from.b) / d + (from.g < from.b ? 1.0f : 0.0f);
			}
			if( hi == from.g ) {
				h = (60.0f/360.0f) * (from.b - from.r) / d + (120.0f/360.0f);
			}
			if( hi == from.b ) {
				h = (60.0f/360.0f) * (from.r - from.g) / d + (240.0f/360.0f);
			}
		}
	}
};

}

#endif  // #ifndef _COLOR_H_
