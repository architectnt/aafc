/*

    Sample modifiers
    2023-2024 Architect Enterprises

*/

#include "aafc.h"
#include "helpers.h"
#include "common.h"
#include <iostream>

static void forceMono(float* input, AAFC_HEADER& header, unsigned char& channels, int& samplelength) {
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
        header.samplelength = samplelength / channels;
        header.channels = 1;
        samplelength /= channels;
        channels = 1;
    }
}

static float* resampleAudio(float* input, AAFC_HEADER& header, int samplerateoverride, int freq, unsigned char channels, int& samplelength) {
    float ratio = (float)samplerateoverride / freq;
    int resampledlen = (int)(samplelength * ratio);

    float* rsmpled = (float*)malloc(resampledlen * sizeof(float));

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
                *(rsmpled + ind) = cubicInterpolate(*(input + y0), *(input + y1), *(input + y2), *(input + y3), mu);
            }
            else {
                break;
            }
        }
    }

    header.freq = samplerateoverride;
    samplelength = resampledlen;
    header.samplelength = resampledlen;

    return rsmpled;
}