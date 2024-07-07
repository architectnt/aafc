/*

    Sample modifiers


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "modifiers.h"

static inline void forceMono(float* input, AAFC_HEADER* header, unsigned char* channels, int* samplelength) {
    if (*channels > 1) {
        int splen = *samplelength / *channels;
        for (int i = 0; i < splen; i++)
        {
            float accu = 0;
            for (int chn = 0; chn < *channels; chn++)
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

static inline float* resampleAudio(float* input, AAFC_HEADER* header, int samplerateoverride, int freq, unsigned char channels, int* samplelength, float pitch) {
    if (pitch == 0) {
        pitch = 1;
    }

    if (samplerateoverride == freq && pitch == 1) {
        return input;
    }

    if (pitch != 1 && samplerateoverride == 0)
        samplerateoverride = freq;

    double ratio = ((double)samplerateoverride / freq) / pitch;

    int splen = *samplelength / channels;
    int resampledlen = (int)(*samplelength * ratio);
    int resampledlenc = (int)(splen * ratio);

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));
    memset(rsmpled, 0, resampledlen);

    for (int ch = 0; ch < channels; ++ch) {
        for (int i = 0; i < resampledlenc; ++i) {
            double oindx = i / ratio;
            int idx0 = (int)oindx;
            int ind = i * channels + ch;

            int y0 = Max(idx0 - 1, 0) * channels + ch;
            int y1 = idx0 * channels + ch;
            int y2 = Min(idx0 + 1, splen - 1) * channels + ch;
            int y3 = Min(idx0 + 2, splen - 1) * channels + ch;
            double mu = oindx - idx0;
            *(rsmpled + ind) = cubic_interpolate(*(input + y0), *(input + y1), *(input + y2), *(input + y3), mu);
        }
    }

    *samplelength = resampledlen;
    if (header != NULL) {
        header->freq = samplerateoverride;
        header->samplelength = resampledlen;
    }

    return rsmpled;
}

static inline float* force_independent_channels(float* input, unsigned char channels, int samplelength) {
    float* output = (float*)malloc(samplelength * sizeof(float));

    int splen = samplelength / channels;
    for (int ch = 0; ch < channels; ch++) {
        for (int i = 0; i < splen; i++) {
            *(output + (i + splen * ch)) = *(input + (i * channels + ch));
        }
    }

    return output;
}

static inline float* normalize(float* input, int len) {
    if (input == NULL || len <= 0) {
        return input; // keep original
    }
    float mx = 0.0f;
    float* ptr;
    for (ptr = input; ptr < input + len; ptr++) {
        if (*ptr > mx) {
            mx = *ptr;
        }
    }

    if (mx < 1e-6) {
        return input; // keep original again
    }

    for (ptr = input; ptr < input + len; ptr++) {
        *ptr /= mx;
    }

    return input;
}

inline float* force_interleave_channels(float* input, unsigned char channels, int samplelength) {
    if (!input || channels <= 0 || samplelength <= 0) {
        return NULL;
    }

    float* output = (float*)malloc(samplelength * sizeof(float));
    if (!output) {
        return NULL;
    }

    int splen = samplelength / channels;
    for (int i = 0; i < splen; i++) {
        for (int ch = 0; ch < channels; ch++) {
            *(output + (i * channels + ch)) = *(input + (i + splen * ch));
        }
    }
    return output;
}