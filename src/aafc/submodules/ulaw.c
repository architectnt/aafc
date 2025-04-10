/*
    
    uLaw

    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "ulaw.h"

unsigned char* encode_ulaw(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    *audsize = h->samplelength;
    unsigned char* const ulaw = (unsigned char*)malloc(h->samplelength);
    unsigned char* uptr = ulaw;

    for (unsigned int i = 0; i < h->samplelength; ptr++, uptr++, i++) {
        short sample = (short)CLAMP(*ptr * 32767.0f, -32768.0f, 32767.0f);
        unsigned char sign = (sample >> 8) & 0x80;
        if (sign != 0) sample = -sample;
        if (sample > CLIP) sample = CLIP;

        sample += BIAS;
        unsigned char exponent = expLut[(sample >> 7) & 0xFF],
            mantissa = (sample >> (exponent + 3)) & 0x0F;
        *uptr = ~(sign | (exponent << 4) | mantissa);
    }
    return ulaw;
}

void decode_ulaw(const unsigned char* input, float* output, const AAFC_HEADER* h) {
    for (const unsigned char* n = input + h->samplelength; input < n; input++, output++) {
        unsigned char smpl = ~(*input);
        signed char sign = (smpl & 0x80) ? -1 : 1;
        unsigned char exponent = (smpl >> 4) & 0x07,
            mantissa = smpl & 0x0F;
        short lin = expLutd[exponent] + (mantissa << (exponent + 3));
        *output = (lin * sign) * INT16_REC;
    }
}
