/*

    Sample modifiers


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "modifiers.h"

void forceMono(float* input, AAFC_HEADER* h) {
    if (h->channels < 2) return;

    const unsigned int splen = h->samplelength / h->channels;
    unsigned char chn;
    float accu;
    const float scale = 1.0f / h->channels;
    for (unsigned int i = 0; i < splen; i++)
    {
        for (chn = 0, accu = 0; chn < h->channels; chn++)
            accu += *(input + (i * h->channels + chn));
        *(input + i) = accu * scale;
    }
    h->samplelength = splen;
    h->channels = 1;
}

float* resampleAudio(float* input, AAFC_HEADER* header, unsigned int samplerateoverride, float pitch, bool nointerp) {
    if (header == NULL) {
        return NULL;
    }

    if (pitch == 0)
        pitch = 1;

    if (samplerateoverride == header->freq && pitch == 1)
        return input;

    if (pitch != 1 && samplerateoverride == 0)
        samplerateoverride = header->freq;

    const double ratio = ((double)samplerateoverride / header->freq) / pitch, iratio = 1.0 / ratio;
    const unsigned int splen = header->samplelength / header->channels,
        resampledlen = (unsigned int)(header->samplelength * ratio),
        resampledlenc = (unsigned int)(splen * ratio);
    unsigned int i, ind, idx0;
    double oindx, mu;

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));
    if (!rsmpled) return input;

    if (nointerp) { // eh whatevs we save the branch predictor anyways
        for (i = 0; i < resampledlenc; i++) {
            ind = (unsigned int)(i * iratio),
                idx0 = (ind < splen) ? ind : splen - 1;

            for (unsigned char ch = 0; ch < header->channels; ch++)
                rsmpled[i * header->channels + ch] = input[idx0 * header->channels + ch];
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

            for (unsigned char ch = 0; ch < header->channels; ch++)
                rsmpled[i * header->channels + ch] = smoothInterpolate(
                    input[i0 * header->channels + ch],
                    input[idx0 * header->channels + ch],
                    input[i1 * header->channels + ch],
                    input[i2 * header->channels + ch],
                    mu
                );
        }
    }

    header->freq = samplerateoverride;
    header->samplelength = resampledlen;

    return rsmpled;
}

float* forceIndependentChannels(float* input, const AAFC_HEADER* h) {
    float* output = (float*)malloc(h->samplelength * sizeof(float));

    const unsigned int splen = h->samplelength / h->channels;
    unsigned int i;
    for (unsigned char ch = 0; ch < h->channels; ch++) {
        for (i = 0; i < splen; i++)
            *(output + (i + splen * ch)) = *(input + (i * h->channels + ch));
    }

    return output;
}

float* normalize(float* input, const AAFC_HEADER* h) {
    if (input == NULL || !h || h->samplelength <= 0)
        return input;

    float mx = 0.0f;
    float* p;
    float* end = input + h->samplelength;
    for (p = input; p < end; p++) {
        const float val = fabsf(*p);
        mx = val > mx ? val : mx;
    }

    if (mx < 1e-6)  return input;

    const float scale = 1.0f / mx;
    for (p = input; p < end; p++) {
        *p *= scale;
    }

    return input;
}

float* forceInterleaveChannels(float* input, const AAFC_HEADER* h) {
    if (!input || h->channels <= 0 || h->samplelength <= 0)
        return NULL;

    float* output = (float*)malloc(h->samplelength * sizeof(float));
    if (!output)
        return NULL;

    unsigned int splen = h->samplelength / h->channels;
    unsigned char ch;
    for (unsigned int i = 0; i < splen; i++) {
        for (ch = 0; ch < h->channels; ch++)
            *(output + (i * h->channels + ch)) = *(input + (i + splen * ch));
    }
    return output;
}