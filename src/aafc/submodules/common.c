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

float lerp(float x0, float x1, float alpha) {
    return x0 * (1 - alpha) + x1 * alpha;
}

float sinc(float x) {
    if (x == 0.0f) {
        return 1.0f;
    }
    double pix = PI * x;
    return sin(pix) / pix;
}

//TODO: fix performance since once exposed performance gets wacky
float sinc_interpolate(float* samples, int sampleCount, double t, int windowSize) {
    float result = 0.0f;

    int start = (int)floor(t) - windowSize / 2;
    int end = start + windowSize;

    for (int i = start; i < end; ++i) {
        float sincValue = sinc(t - (double)i);
        int cind = Clamp(i, 0, sampleCount);
        result += *(samples + cind) * sincValue;
    }

    return result;
}

float cubic_interpolate(float y0, float y1, float y2, float y3, double mu) {
    float a0, a1, a2, a3, mu2;

    mu2 = mu * mu;
    a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
    a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    a2 = -0.5f * y0 + 0.5f * y2;
    a3 = y1;

    return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
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
    }
    else {
        significand += 0.5f;
    }

    float result = pow(2, exponent) * significand;
    return sign ? -result : result;
}

bool header_valid(const unsigned char* bytes) {
    return *bytes == 'A' && *(bytes + 1) == 'A' && *(bytes + 2) == 'F' && *(bytes + 3) == 'C' && *((int*)bytes + 2) <= AAFCVERSION;
}

bool aftheader_valid(const unsigned char* bytes) {
    return *bytes == 'A' && *(bytes + 1) == 'F' && *(bytes + 2) == 'T' && *((int*)bytes + 2) <= AAFCVERSION;
}

// this is what happens when you make A SINGLE STRUCT optional in your code.
AAFC_HEADER* create_header(unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype) {
    AAFC_HEADER* h = (AAFC_HEADER*)malloc(sizeof(AAFC_HEADER));
    if (h == NULL) return NULL;

    strncpy(h->headr, AAFC_STRING, 5); // ignore what windows says about strncpy (ms is anti cross-platform back then)
    h->version = AAFCVERSION;
    h->freq = freq;
    h->channels = channels;
    h->samplelength = samplelength;
    h->bps = bps;
    h->sampletype = sampletype;

    return h;
}

bool create_aftheader(AAFCFILETABLE* t) {
    if (t == NULL) {
        return false;
    }

    strncpy(t->headr, AFT_STRING, 4);
    t->version = AAFCVERSION;

    return true;
}