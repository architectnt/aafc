/*
    
    uLaw

    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "ulaw.h"

unsigned char* encode_ulaw(float* ptr, unsigned int samplelength, size_t* audsize) {
    unsigned char* ulaw = (unsigned char*)malloc(samplelength);
    unsigned char* uptr = ulaw;

    const short* explut = expLut;

    for (unsigned int i = 0; i < samplelength; ptr++, uptr++, i++) {
        short sample = (short)clampf(*ptr * 32767.0f, -32768.0f, 32767.0f);
        short sign = (sample >> 8) & 0x80;
        if (sign != 0) sample = -sample;
        if (sample > CLIP) sample = CLIP;

        sample += BIAS;
        short exponent = *(explut + ((sample >> 7) & 0xFF));
        short mantissa = (sample >> (exponent + 3)) & 0x0F;
        *uptr = ~(sign | (exponent << 4) | mantissa);
    }

    *audsize = samplelength;
    return ulaw;
}

void decode_ulaw(const unsigned char* input, float* output, const unsigned int sampleCount) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
    const short* explut = expLutd;

    for (const unsigned char* n = smpraw + sampleCount; smpraw < n; smpraw++, output++) {
        unsigned char smpl = ~(*smpraw);
        short sign = (smpl & 0x80);
        short exponent = (smpl >> 4) & 0x07;
        short mantissa = smpl & 0x0F;
        short lin = *(explut + exponent) + (mantissa << (exponent + 3));
        lin = !sign ? lin : -lin;

        *output = lin * INT16_REC;
    }
}
