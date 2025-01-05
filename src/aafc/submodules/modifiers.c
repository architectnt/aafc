/*

    Sample modifiers


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "modifiers.h"

void forceMono(float* input, AAFC_HEADER* header, unsigned char* channels, unsigned int* samplelength) {
    if (*channels < 2) return;

    const unsigned int splen = *samplelength / *channels;
    unsigned char chn;
    float accu;
    const float scale = 1.0f / *channels;
    for (unsigned int i = 0; i < splen; i++)
    {
        for (chn = 0, accu = 0; chn < *channels; chn++)
            accu += *(input + (i * *channels + chn));
        *(input + i) = accu * scale;
    }
    header->samplelength = splen;
    header->channels = 1;
    *samplelength = splen;
    *channels = 1;
}

float* resampleAudio(float* input, AAFC_HEADER* header, unsigned int samplerateoverride, unsigned int freq, unsigned char channels, unsigned int* samplelength, float pitch, bool nointerp) {
    if (pitch == 0)
        pitch = 1;

    if (samplerateoverride == freq && pitch == 1)
        return input;

    if (pitch != 1 && samplerateoverride == 0)
        samplerateoverride = freq;

    const double ratio = ((double)samplerateoverride / freq) / pitch, iratio = 1.0 / ratio;
    const unsigned int splen = *samplelength / channels, 
        resampledlen = (unsigned int)(*samplelength * ratio),
        resampledlenc = (unsigned int)(splen * ratio);
    unsigned int i, ind, idx0;
    double oindx, mu;

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));
    if (!rsmpled) return input;

    if (nointerp) { // eh whatevs we save the branch predictor anyways
        for (i = 0; i < resampledlenc; i++) {
            ind = (unsigned int)(i * iratio),
                idx0 = (ind < splen) ? ind : splen - 1;

            for (unsigned char ch = 0; ch < channels; ch++)
                rsmpled[i * channels + ch] = input[idx0 * channels + ch];
        }
    }
    else {
        for (i = 0; i < resampledlenc; i++) {
            oindx = i * iratio;
            idx0 = (unsigned int)oindx;
            mu = oindx - idx0;

            const unsigned int i0 = (idx0 > 0 ? idx0 - 1 : 0),
                i1 = (idx0 + 1 < splen ? idx0 + 1 : splen - 1),
                i2 = (idx0 + 2 < splen ? idx0 + 2 : splen - 1);

            for (unsigned char ch = 0; ch < channels; ch++)
                rsmpled[i * channels + ch] = smooth_interpol(
                    input[i0 * channels + ch],
                    input[idx0 * channels + ch],
                    input[i1 * channels + ch],
                    input[i2 * channels + ch],
                    mu
                );
        }
    }

    *samplelength = resampledlen;
    if (header != NULL) {
        header->freq = samplerateoverride;
        header->samplelength = resampledlen;
    }

    return rsmpled;
}

float* force_independent_channels(float* input, const unsigned char channels, const unsigned int samplelength) {
    float* output = (float*)malloc(samplelength * sizeof(float));

    const unsigned int splen = samplelength / channels;
    unsigned int i;
    for (unsigned char ch = 0; ch < channels; ch++) {
        for (i = 0; i < splen; i++)
            *(output + (i + splen * ch)) = *(input + (i * channels + ch));
    }

    return output;
}

float* normalize(float* input, const unsigned int len) {
    if (input == NULL || len <= 0)
        return input;

    float mx = 0.0f;
    float* ptr;
    for (ptr = input; ptr < input + len; ptr++) {
        if (fabsf(*ptr) > mx)
            mx = fabsf(*ptr);
    }

    if (mx < 1e-6) 
        return input;

    for (ptr = input; ptr < input + len; ptr++) {
        *ptr /= mx;
    }

    return input;
}

float* force_interleave_channels(float* input, const unsigned char channels, const unsigned int samplelength) {
    if (!input || channels <= 0 || samplelength <= 0)
        return NULL;

    float* output = (float*)malloc(samplelength * sizeof(float));
    if (!output)
        return NULL;

    unsigned int splen = samplelength / channels;
    unsigned char ch;
    for (unsigned int i = 0; i < splen; i++) {
        for (ch = 0; ch < channels; ch++)
            *(output + (i * channels + ch)) = *(input + (i + splen * ch));
    }
    return output;
}