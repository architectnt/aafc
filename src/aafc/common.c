/*

    common functions


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/


#include "aafc.h"
#include "common.h"

void compactHeader(unsigned char* dt, AAFC_HEADER h) {
    unsigned char* p = dt;

    // SIGNATURE
    *(unsigned short*)p = h.signature; p += 2;
    *(unsigned short*)p = h.version; p += 2;

    // FREQ
    *p++ = (h.freq & 0xFF);
    *p++ = ((h.freq >> 8) & 0xFF);
    *p++ = ((h.freq >> 16) & 0xFF);

    // DEFS
    *p++ = (h.channels & 0x0F) | (h.sampletype & 0x0f) << 4;
    *p++ = h.bps;

    // SAMPLE LENGTH & LOOP POINTS
    *(unsigned int*)p = h.samplelength; p += 4;
    *(unsigned int*)p = h.loopst; p += 4;
    *(unsigned int*)p = h.loopend;
}

AAFC_HEADER parseHeader(const unsigned char* bytes, unsigned char* offset) {
    if (bytes == NULL)
        return (AAFC_HEADER) {0};

    const unsigned short sig = *(unsigned short*)bytes;
    const unsigned short ver = *((unsigned short*)bytes + 1);

    if (sig == AAFC_SIGNATURE && ver > 300 && ver <= AAFCVERSION) {
        if(offset) *offset = HEADERSIZE;
        const unsigned char* p = bytes;
        AAFC_HEADER h = { 0 };
        h.signature = *(unsigned short*)p; p += 2;
        h.version = *(unsigned short*)p; p += 2;
        h.freq = *p | (p[1] << 8) | (p[2] << 16); p += 3;

        h.channels = *p & 0x0F;
        h.sampletype = (*p >> 4) & 0x0F;
        h.bps = *++p;

        h.samplelength = *(unsigned int*)++p; p += 4;
        h.loopst = *(unsigned int*)p; p += 4;
        h.loopend = *(unsigned int*)p;
        return h;
    }
    else if (sig == AAFC_SIGNATURE && ver <= 300) {
        if (offset) *offset = sizeof(AAFC_HEADER);
        return *(AAFC_HEADER*)bytes;
    }
    else if (*(unsigned int*)bytes == (unsigned int)LEGACYHEADER
        && *((unsigned int*)bytes + 2) <= AAFCVERSION) 
    {
        if (offset) *offset = sizeof(AAFC_LCHEADER);
        const AAFC_LCHEADER lh = *(AAFC_LCHEADER*)bytes;
        return (AAFC_HEADER) {
            AAFC_SIGNATURE, (unsigned short)lh.version,
                lh.freq,
                lh.channels, lh.bps, lh.sampletype,
                lh.samplelength, 0, 0
        };
    }
    return (AAFC_HEADER) {0};
}

float smoothInterpolate(float y0, float y1, float y2, float y3, double t) {
    float c1 = 0.5f * (y2 - y0),
        c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3,
        c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * t + c2) * t + c1) * t + y1;
}

unsigned char minifloat(float val) {
    unsigned int eger = *(unsigned int*)&val; // HAHhaHAhahHAhhh..hah...heh..
    int exp = ((eger >> 23) & 0xFF) - 121;
    exp = exp < 0 ? 0 : (exp > 15 ? 15 : exp);
    unsigned char fr = (eger >> 20) & 0x07;
    return ((eger >> 31) << 7) | (exp << 3) | fr;
}

float dminif(unsigned char val) {
    int stexp = (val >> 3) & 0xF;
    int mt = val & 0x07;
    int exp;
    float sig;
    if(stexp == 0){
        exp = -6;
        sig = mt / 8.0f;
    }else{
        exp = stexp - 6;
        sig = 1.0f + mt / 8.0f;
    }
    float result = ldexp(sig, exp);
    return (val & 0x80) ? -result : result;
}

float dhalf(unsigned short val) {
    int exponent = ((val >> 10) & 0x1F) - 15;
    float significand = (val & 0x3FF) / 1024.0f + 1.0f;
    float result = pow(2, exponent) * significand;
    return ((val >> 15) & 1) ? -result : result;
}

unsigned short halfpercision(float val) { // stupidly inefficient
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