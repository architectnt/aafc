/*

    Sample modifiers
    2023-2024 Architect Enterprises

*/

#include "aafc.h"
#include "helpers.h"
#include "common.h"
#include <iostream>

static void forceMono(float* input, AAFC_HEADER* header, unsigned char& channels, int& samplelength) {
    if (channels > 1) {
        for (int i = 0; i < samplelength; i++)
        {
            float msmpl = 0;
            for (int chn = 0; chn < channels; chn++)
            {
                int ind = i * channels + chn;
                if (ind < samplelength)
                {
                    msmpl += *(input + ind);
                }
            }
            *(input + i) = msmpl / channels;
        }
        header->samplelength = samplelength / channels;
        header->channels = 1;
        samplelength /= channels;
        channels = 1;
    }
}

static float* resampleAudio(float* input, AAFC_HEADER* header, int samplerateoverride, int freq, unsigned char channels, int& samplelength) {
    float ratio = (float)samplerateoverride / freq;
    int resampledlen = (int)(samplelength * ratio);

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));
    memset(rsmpled, 0, resampledlen);

    for (int ch = 0; ch < channels; ++ch) {
        for (int i = 0; i < resampledlen; ++i) {
            double oindx = i / ratio;
            int idx0 = (int)oindx;
            int ind = i * channels + ch;
            if (ind < resampledlen) {
                int y0 = Max(idx0 - 1, 0) * channels + ch;
                int y1 = idx0 * channels + ch;
                int y2 = Min(idx0 + 1, samplelength - 1) * channels + ch;
                int y3 = Min(idx0 + 2, samplelength - 1) * channels + ch;
                double mu = oindx - idx0;
                *(rsmpled + ind) = cubic_interpolate(*(input + y0), *(input + y1), *(input + y2), *(input + y3), mu);
            }
            else {
                break;
            }
        }
    }

    samplelength = resampledlen;
    if (header != nullptr) {
        header->freq = samplerateoverride;
        header->samplelength = resampledlen;
    }

    return rsmpled;
}

static float* force_independent_channels(float* input, unsigned char channels, int samplelength) {
    float* output = (float*)malloc(samplelength * sizeof(float));

    int splen = samplelength / channels;
    for (int ch = 0; ch < channels; ch++) {
        for (int i = 0; i < splen; i++) {
            *(output + (i + splen * ch)) = *(input + (i * channels + ch));
        }
    }

    return output;
}

static float* force_interleave_channels(float* input, unsigned char channels, int samplelength) {
    if (!input || channels <= 0 || samplelength <= 0) {
        return nullptr;
    }

    float* output = (float*)malloc(samplelength * sizeof(float));
    if (!output) {
        return nullptr;
    }

    int splen = samplelength / channels;
    for (int i = 0; i < splen; i++) {
        for (int ch = 0; ch < channels; ch++) {
            *(output + (i * channels + ch)) = *(input + (i + splen * ch));
        }
    }
    return output;
}