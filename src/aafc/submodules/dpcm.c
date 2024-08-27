/*

    Single bit Delta PCM


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "dpcm.h"

unsigned char* encode_dpcm(float* ptr, unsigned int samplelength, size_t* audsize) {
    size_t bsize = ((size_t)samplelength + 7) / 8;
    unsigned char* dpcm = (unsigned char*)malloc(bsize);
    memset(dpcm, 0, bsize);

    unsigned char accum = 63;
    for (unsigned int i = 0; i < samplelength; ptr++, i++) {
        unsigned char next = (unsigned char)((*ptr + 1.0f) * 63.5f);
        if (next > accum) {
            *(dpcm + (i >> 3)) |= 1 << (i & 7);
            accum++;
        }
        else {
            *(dpcm + (i >> 3)) &= ~(1 << (i & 7));
            accum--;
        }

        if (accum > 127) accum = 127;
        if (accum < 0) accum = 0;
    }

    *audsize = bsize;
    return dpcm;
}

void decode_dpcm(const unsigned char* input, float* output, const unsigned int sampleCount) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
    signed char accum = 0;
    for (unsigned int i = 0; i < sampleCount; i++) {
        accum += ((*(smpraw + (i >> 3)) >> (i & 7)) & 1) ? 1 : -1;
        if (accum > 63) accum = 63;
        if (accum < -64) accum = -64;
        *output++ = accum * INT7_REC;
    }
}