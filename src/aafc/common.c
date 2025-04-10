/*

    common functions


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/


#include <aafc.h>

float dhalf(unsigned short val) {
    if (val == 0) return 0.0f;

    int sign = (val >> 15) & 0x1;
    int exponent = ((val >> 10) & 0x1F) - 15;
    float significand = (val & 0x3FF) / 1024.0f + 1.0f;

    float result = pow(2, exponent) * significand;
    return sign ? -result : result;
}

float smoothInterpolate(float y0, float y1, float y2, float y3, double t) {
    float c1 = 0.5f * (y2 - y0),
        c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3,
        c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * t + c2) * t + c1) * t + y1;
}

unsigned char minifloat(float val) {
    if (val == 0.0f) return 0;
    unsigned int bits = *(unsigned int*)&val;
    unsigned int sgn = (bits >> 31) & 0x01;
    int exp = ((bits >> 23) & 0xFF) - 127 + 6;
    exp = (exp > 15) ? 15 : (exp < 0) ? 0 : exp;
    unsigned int fraction;
    if (exp < 0) {
        fraction = (bits >> (23 + exp)) & 0x07;
        exp = 0;
    }
    else {
        exp = (exp > 15) ? 15 : exp;
        fraction = (bits >> (23 - 3)) & 0x07;
    }
    return (sgn << 7) | (exp << 3) | fraction;
}

unsigned short halfpercision(float val) {
    unsigned int eger = *(unsigned int*)&val; // int eger (humor)
    unsigned int sgn = (eger >> 31) & 0x1;
    int exp = ((eger >> 23) & 0xFF) - 127 + 15;
    int mantissa = eger & 0x7FFFFF;
    int overflow = (exp >= 31) ? 1 : 0;
    int underflow = (exp <= 0) ? 1 : 0;
    mantissa = overflow * ((eger == 0x7F800000) ? 0 : 0x3FF) + (1 - overflow) * mantissa;
    exp = overflow * 31 + (1 - overflow) * exp;

    int shiftAmount = 1 - exp;
    shiftAmount = (shiftAmount > 0) ? shiftAmount : 0;
    mantissa = underflow * ((mantissa | 0x800000) >> shiftAmount) + (1 - underflow) * mantissa;
    exp *= (1 - underflow);

    unsigned int rmants = (mantissa + 0x7FF + ((mantissa >> 13) & 1)) >> 13;
    exp = (rmants == 0x400) ? (exp + 1) : exp;
    rmants &= 0x3FF;
    return (sgn << 15) | (exp << 10) | (rmants);
}

float dminif(unsigned char val) {
    if (val == 0) return 0.0f;
    int sign = (val >> 7) & 0x1;
    int exponent = ((val >> 3) & 0xF) - 6;
    float significand = (val & 0x07) / 8.0f + 0.5f;

    if (((val >> 3) & 0xF) == 0) {
        significand /= 8.0f;
    } else {
        significand += 0.5f;
    }

    float result = pow(2, exponent) * significand;
    return sign ? -result : result;
}

bool header_valid(const unsigned char* bytes) {
    return bytes != NULL && *(unsigned short*)bytes == AAFC_SIGNATURE && *((unsigned short*)bytes + 1) <= AAFCVERSION;
}

bool legacy_header_valid(const unsigned char* bytes) {
    return bytes != NULL && *(const unsigned int*)bytes == (unsigned int)LEGACYHEADER && *((unsigned int*)bytes + 2) <= AAFCVERSION;
}