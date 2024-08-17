/*

    IMA-ADPCM 4-bit encoding & decoding optimized implementation


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "adpcm.h"

signed char* encode_adpcm(float* ptr, unsigned int samplelength, size_t* audsize) {
    size_t bsize = samplelength / 2;

    signed char* adpcm_base = (signed char*)malloc(bsize);
    signed char* adpcm = adpcm_base;
    const short* stptr = adpcm_step_size_table;
    const signed char* itbptr = adpcm_index_table;

    short sample = 0;
    signed char index = 0;
    short step;
    signed char delta;
    int diff;
    int valpred = 0;
    int vpdiff;
    unsigned char bufferstep;
    signed char outputbuffer;
    signed char sign;

    step = *stptr;

    bufferstep = 1;

    for (unsigned int i = 0; i < samplelength; ptr++, i++) {
        sample = (short)clampf(*ptr * 32767.0f, -32768.0f, 32767.0f);

        diff = sample - valpred;
        sign = (diff < 0) ? 8 : 0;
        if (sign) diff = (-diff);

        delta = 0;
        vpdiff = (step >> 3);

        if (diff >= step) {
            delta = 4;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if (diff >= step) {
            delta |= 2;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if (diff >= step) {
            delta |= 1;
            vpdiff += step;
        }

        if (sign)
            valpred -= vpdiff;
        else
            valpred += vpdiff;

        if (valpred > 32767)
            valpred = 32767;
        else if (valpred < -32768)
            valpred = -32768;

        delta |= sign;

        index += *(itbptr + delta);
        if (index < 0) index = 0;
        if (index > 88) index = 88;
        step = *(stptr + index);

        if (bufferstep) {
            outputbuffer = (delta << 4) & 0xf0;
        }
        else {
            *adpcm++ = (delta & 0x0f) | outputbuffer;
        }

        bufferstep = ~bufferstep & 0x01;
    }

    *audsize = bsize;
    return adpcm_base;
}

void decode_adpcm(const unsigned char* input, float* output, const unsigned int sampleCount) {
    const signed char* adpcm = (const signed char*)(input + sizeof(AAFC_HEADER));
    const short* stptr = adpcm_step_size_table;
    const signed char* itbptr = adpcm_index_table;

    signed char index = 0;
    short step = *stptr;
    signed char delta;
    int valpred = 0;
    int vpdiff;
    unsigned char bufferstep = 0;
    int inputbuffer = 0;
    signed char sign;

    for (const float* n = output + sampleCount; output < n; output++) {
        if (bufferstep) {
            delta = inputbuffer & 0xf;
        }
        else {
            inputbuffer = *adpcm++;
            delta = (inputbuffer >> 4) & 0xf;
        }
        bufferstep = ~bufferstep & 0x01;

        index += *(itbptr + delta);
        if (index < 0) index = 0;
        if (index > 88) index = 88;

        sign = delta & 8;
        delta = delta & 7;

        vpdiff = step >> 3;
        if (delta & 4) vpdiff += step;
        if (delta & 2) vpdiff += step >> 1;
        if (delta & 1) vpdiff += step >> 2;

        if (sign)
            valpred -= vpdiff;
        else
            valpred += vpdiff;

        if (valpred > 32767)
            valpred = 32767;
        else if (valpred < -32768)
            valpred = -32768;

        step = *(stptr + index);

        *output = valpred * INT16_REC;
    }
}