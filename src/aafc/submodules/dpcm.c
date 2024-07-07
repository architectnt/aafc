/*

    Single bit Delta PCM


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "dpcm.h"

inline unsigned char* encode_dpcm(float* ptr, int samplelength, size_t* audsize) {
    int bytesize = (samplelength + 7) / 8;
    unsigned char* dpcm_base = (unsigned char*)malloc(bytesize);
    memset(dpcm_base, 0, bytesize);

    float prevsample = 0;
    float dlt;
    int b;
    bool alternate = false;

    int windowSize = 100;
    float sumamp = 0;
    float avgamp = 0;
    float threshold = 0.0256;
    float dynamicThreshold;

    for (int i = 0; i < samplelength; i++) {
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
            alternate = (dlt >= 0) ? false : true;
        }

        *(dpcm_base + (i / 8)) |= b << (i % 8);
        prevsample += ((b == 0) ? -threshold : threshold);
    }

    *audsize = bytesize;
    return dpcm_base;
}

inline void decode_dpcm(const unsigned char* input, float* output, int sampleCount) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
    float prevsmpl = 0;
    float delta = 0.0256;
    for (int i = 0; i < sampleCount; i++) {
        int b = (*(smpraw + (i / 8)) >> (i % 8)) & 1;
        prevsmpl += !b ? -delta : delta;
        *output++ = Clamp(prevsmpl, -1.0, 1.0);
    }
}