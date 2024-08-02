/*

    Sample modifiers


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "modifiers.h"

inline void forceMono(float* input, AAFC_HEADER* header, unsigned char* channels, unsigned int* samplelength) {
    if (*channels > 1) {
        const unsigned int splen = *samplelength / *channels;
        unsigned char chn;
        float accu;
        for (unsigned int i = 0; i < splen; i++)
        {
            for (chn = 0, accu = 0; chn < *channels; chn++)
            {
                accu += *(input + (i * *channels + chn));
            }
            *(input + i) = accu / *channels;
        }
        header->samplelength = splen;
        header->channels = 1;
        *samplelength /= *channels;
        *channels = 1;
    }
}

inline float* resampleAudio(float* input, AAFC_HEADER* header, unsigned int samplerateoverride, unsigned int freq, unsigned char channels, unsigned int* samplelength, float pitch) {
    if (pitch == 0)
        pitch = 1;

    if (samplerateoverride == freq && pitch == 1)
        return input;

    if (pitch != 1 && samplerateoverride == 0)
        samplerateoverride = freq;

    const double ratio = ((double)samplerateoverride / freq) / pitch;
    const unsigned int splen = *samplelength / channels;
    const unsigned int resampledlen = (unsigned int)(*samplelength * ratio);
    const unsigned int resampledlenc = (unsigned int)(splen * ratio);
    unsigned int i, ind, idx0, y0, y1, y2, y3;
    double oindx, mu;

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));

    for (unsigned char ch = 0; ch < channels; ch++) {
        for (i = 0; i < resampledlenc; i++) {
            oindx = i / ratio;
            idx0 = (unsigned int)oindx;
            ind = i * channels + ch;

            y0 = (idx0 > 0 ? idx0 - 1 : 0) * channels + ch;
            y1 = idx0 * channels + ch;
            y2 = (idx0 + 1 < splen ? idx0 + 1 : splen - 1) * channels + ch;
            y3 = (idx0 + 2 < splen ? idx0 + 2 : splen - 1) * channels + ch;
            mu = oindx - idx0;
            *(rsmpled + ind) = smooth_interpol(*(input + y0), *(input + y1), *(input + y2), *(input + y3), mu);
        }
    }

    *samplelength = resampledlen;
    if (header != NULL) {
        header->freq = samplerateoverride;
        header->samplelength = resampledlen;
    }

    return rsmpled;
}

inline float* force_independent_channels(float* input, const unsigned char channels, const unsigned int samplelength) {
    float* output = (float*)malloc(samplelength * sizeof(float));

    const unsigned int splen = samplelength / channels;
    unsigned int i;
    for (unsigned char ch = 0; ch < channels; ch++) {
        for (i = 0; i < splen; i++) {
            *(output + (i + splen * ch)) = *(input + (i * channels + ch));
        }
    }

    return output;
}

inline float* normalize(float* input, const unsigned int len) {
    if (input == NULL || len <= 0)
        return input;

    float mx = 0.0f;
    float* ptr;
    for (ptr = input; ptr < input + len; ptr++) {
        if (*ptr > mx)
            mx = *ptr;
    }

    if (mx < 1e-6) 
        return input;

    for (ptr = input; ptr < input + len; ptr++) {
        *ptr /= mx;
    }

    return input;
}

inline float* force_interleave_channels(float* input, const unsigned char channels, const unsigned int samplelength) {
    if (!input || channels <= 0 || samplelength <= 0)
        return NULL;

    float* output = (float*)malloc(samplelength * sizeof(float));
    if (!output)
        return NULL;

    unsigned int splen = samplelength / channels;
    unsigned char ch;
    for (unsigned int i = 0; i < splen; i++) {
        for (ch = 0; ch < channels; ch++) {
            *(output + (i * channels + ch)) = *(input + (i + splen * ch));
        }
    }
    return output;
}