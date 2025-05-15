/*

    Single bit Delta PCM


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "dpcm.h"
#include "../helpers.h"
#include "../common.h"

unsigned char* encode_dpcm(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    *audsize = ((size_t)h->samplelength + 7) / 8;
    unsigned char* const dpcm = (unsigned char*)malloc(*audsize);
    memset(dpcm, 0, *audsize);

    unsigned char sum = 63;
    for (unsigned int i = 0; i < h->samplelength; ptr++, i++) {
        unsigned char next = (unsigned char)(CLAMP((*ptr + 1.0f) * 63.5f, 0.0f, 255.0f));
        if (next > sum) {
            *(dpcm + (i >> 3)) |= 1 << (i & 7);
            sum++;
        }
        else {
            *(dpcm + (i >> 3)) &= ~(1 << (i & 7));
            sum--;
        }

        sum = sum < 0 ? 0 : sum > 0x7F ? 0x7F : sum;
    }
    return dpcm;
}

void decode_dpcm(const unsigned char* input, float* output, const AAFC_HEADER* h) {
    signed char sum = 0;
    for (unsigned int i = 0; i < h->samplelength; i++) {
        sum += ((*(input + (i >> 3)) >> (i & 7)) & 1) ? 1 : -1;
        sum = sum < -0x40 ? -0x40 : sum > 0x3F ? 0x3F : sum;
        *output++ = sum * INT7_REC;
    }
}