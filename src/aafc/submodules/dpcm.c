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

    float prevsample = 0;
    float dlt;
    unsigned char b;
    bool alternate = false;

    int windowSize = 100;
    float sumamp = 0;
    float avgamp = 0;
    float threshold = 0.0256;
    float dynamicThreshold;

    for (unsigned int i = 0; i < samplelength; i++) {
        float sample = *(ptr + i);

        sumamp += sample;
        if (i >= windowSize) {
            sumamp -= fabs(*(ptr + i - windowSize));
            avgamp = sumamp / windowSize;
            dynamicThreshold = threshold + avgamp;
        }
        else {
            dynamicThreshold = threshold;
        }

        dlt = sample - prevsample;

        if (fabs(dlt) < dynamicThreshold) {
            b = alternate ? 1 : 0;
            alternate = !alternate;
        }
        else {
            b = (dlt >= 0) ? 1 : 0;
            alternate = !b;
        }

        *(dpcm + (i / 8)) |= b << (i % 8);
        prevsample += ((b == 0) ? -threshold : threshold);
    }

    *audsize = bsize;
    return dpcm;
}

void decode_dpcm(const unsigned char* input, float* output, const unsigned int sampleCount) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
    float prevsmpl = 0;
    float delta = 0.0256;
    unsigned char b = 0;
    for (unsigned int i = 0; i < sampleCount; i++) {
        b = (*(smpraw + (i / 8)) >> (i % 8)) & 1;
        prevsmpl += !b ? -delta : delta;
        *output++ = clampf(prevsmpl, -1.0, 1.0);
    }
}