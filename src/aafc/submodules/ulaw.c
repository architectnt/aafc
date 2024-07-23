/*
    
    uLaw

    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "ulaw.h"

inline unsigned char* encode_ulaw(float* ptr, unsigned int samplelength, size_t* audsize) {
    unsigned char* ulaw_base = (unsigned char*)malloc(samplelength * sizeof(unsigned char));

    unsigned char* ulaw = ulaw_base;
    const short* explut = exp_lut;

    for (unsigned int i = 0; i < samplelength; ptr++, ulaw++, i++) {
        short sample = (short)Clamp(*ptr * 32767.0f, -32768.0f, 32767.0f);
        short sign = (sample >> 8) & 0x80;
        if (sign != 0) sample = -sample;
        if ((short)sample > 32635) sample = 32635;

        sample = sample + 0x84;
        short exponent = *(explut + ((sample >> 7) & 0xFF));
        short mantissa = (sample >> (exponent + 3)) & 0x0F;
        unsigned char bt = ~(sign | (exponent << 4) | mantissa);

        *ulaw = bt;
    }

    *audsize = samplelength;
    return ulaw_base;
}

inline void decode_ulaw(const unsigned char* input, float* output, unsigned int sampleCount) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
    const short* explut = exp_lutd;

    for (unsigned int i = 0; i < sampleCount; smpraw++, output++, i++) {
        unsigned char smpl = ~(*smpraw);
        short sign = (smpl & 0x80);
        short exponent = (smpl >> 4) & 0x07;
        short mantissa = smpl & 0x0F;
        short lin = *(explut + exponent) + (mantissa << (exponent + 3));
        if (sign != 0) lin = -lin;

        *output = lin * INT16_REC;
    }
}
