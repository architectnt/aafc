/*

    Single bit Delta PCM


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "dpcm.h"

unsigned char* encode_dpcm(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    *audsize = ((size_t)h->samplelength + 7) / 8;
    unsigned char* const dpcm = (unsigned char*)malloc(*audsize);
    memset(dpcm, 0, *audsize);

    unsigned char accum = 63;
    for (unsigned int i = 0; i < h->samplelength; ptr++, i++) {
        unsigned char next = (unsigned char)(CLAMP((*ptr + 1.0f) * 63.5f, 0.0f, 255.0f));
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
    return dpcm;
}

void decode_dpcm(const unsigned char* input, float* output, const AAFC_HEADER* h) {
    signed char accum = 0;
    for (unsigned int i = 0; i < h->samplelength; i++) {
        accum += ((*(input + (i >> 3)) >> (i & 7)) & 1) ? 1 : -1;
        if (accum > 63) accum = 63;
        if (accum < -64) accum = -64;
        *output++ = accum * INT7_REC;
    }
}