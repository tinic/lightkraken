#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "./ftoa.h"

int ftoa(char *s, float f, int *status) {

#define _FTOA_TOO_LARGE 1
#define _FTOA_TOO_SMALL -1

    if (status) {
        *status = 0;
    }
    if (f == 0.0f) {
        s[0] = '0';
        s[1] = '.';
        s[2] = '0';
        s[3] = 0;
        return strlen(s);
    }
    union {
        long L;
        float F;
    } x;
    x.F = f;
    int32_t mantissa = (x.L & 0xFFFFFF) | 0x800000;
    int32_t frac_part = 0;
    int32_t int_part = 0;
    int16_t exp2 = int16_t((uint8_t)(x.L >> 23)) - 127;
    if (exp2 >= 31) {
        s[0] = '0';
        s[1] = '.';
        s[2] = '0';
        s[3] = 0;
        if (status) {
            *status = _FTOA_TOO_LARGE;
        }
        return 0;
    } else if (exp2 < -23) {
        s[0] = '0';
        s[1] = '.';
        s[2] = '0';
        s[3] = 0;
        if (status) {
            *status = _FTOA_TOO_SMALL;
        }
        return 0;
    } else if (exp2 >= 23) {
        int_part = mantissa << (exp2 - 23);
    } else if (exp2 >= 0) {
        int_part = mantissa >> (23 - exp2);
        frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
    } else { /* if (exp2 < 0) */
        frac_part = (mantissa & 0xFFFFFF) >> -(exp2 + 1);
    }
    char *p = s;
    if (x.F < 0) {
        *p++ = '-';
    }
    if (int_part == 0) {
        *p++ = '0';
    } else {
        p += sprintf(p, "%d", int(int_part));
    }
    *p++ = '.';
    if (frac_part == 0) {
        *p++ = '0';
    } else {
        for (int32_t m = 0; m < 32; m++) {
            frac_part = (frac_part << 3) + (frac_part << 1);
            *p++ = (frac_part >> 24) + '0';
            frac_part &= 0xFFFFFF;
        }
        for (--p; p[0] == '0' && p[-1] != '.'; --p) { }
        ++p;
    }
    *p = 0;
    return strlen(s);
}
