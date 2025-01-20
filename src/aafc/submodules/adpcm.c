/*

    IMA-ADPCM 4-bit encoding & decoding optimized implementation


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "adpcm.h"

signed char* encode_adpcm(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    *audsize = h->samplelength / 2;
    signed char* const adpcm_base = (signed char*)malloc(*audsize);
    signed char* adpcm = adpcm_base;
    short sample = 0, step = adpcmStepSize[0];
    signed char idx = 0, delta, byte = 0, sign;
    int diff, pred = 0, vpd;

    for (unsigned int i = 0; i < h->samplelength; ptr++, i++) {
        sample = (short)CLAMP(*ptr * 32767.0f, -32768.0f, 32767.0f);

        diff = sample - pred;
        sign = (diff < 0) ? 8 : 0;
        if (sign) diff = (-diff);

        delta = 0;
        vpd = (step >> 3);

        if (diff >= step) {
            delta = 4;
            diff -= step;
            vpd += step;
        }
        step >>= 1;
        if (diff >= step) {
            delta |= 2;
            diff -= step;
            vpd += step;
        }
        step >>= 1;
        if (diff >= step) {
            delta |= 1;
            vpd += step;
        }

        pred += sign ? -vpd : vpd;
        pred = pred > 32767 ? 32767 : pred < -32768 ? -32768 : pred;

        delta |= sign;

        idx += adpcmIndexTable[delta];
        idx = idx < 0 ? 0 : idx > 88 ? 88 : idx;
        step = adpcmStepSize[idx];

        if (i & 1) *adpcm++ = (delta & 0x0f) | byte;
        else byte = (delta << 4) & 0xf0;
    }
    return adpcm_base;
}

void decode_adpcm(const unsigned char* input, float* output, const AAFC_HEADER* h) {
    const signed char* adpcm = (const signed char*)input;

    short step = adpcmStepSize[0];
    int pred = 0, vpd;
    unsigned char alt = 0;
    signed char byte = 0, sign, delta = 0, idx = 0;

    for (float* const n = output + h->samplelength; output < n; output++) {
        delta = alt ? byte & 0xf : ((byte = *adpcm++) >> 4) & 0xf;
        alt ^= 1;

        idx += adpcmIndexTable[delta];
        idx = idx < 0 ? 0 : idx > 88 ? 88 : idx;

        sign = delta & 8;
        delta = delta & 7;

        vpd = step >> 3;
        if (delta & 4) vpd += step;
        if (delta & 2) vpd += step >> 1;
        if (delta & 1) vpd += step >> 2;

        pred += sign ? -vpd : vpd;
        pred = pred > 32767 ? 32767 : pred < -32768 ? -32768 : pred;

        step = adpcmStepSize[idx];

        *output = pred * INT16_REC;
    }
}