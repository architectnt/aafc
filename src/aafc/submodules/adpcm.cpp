/*

    IMA-ADPCM 4-bit encoding & decoding optimized implementation
    2023-2024 Architect Enterprises

*/


#include "helpers.h"
#include "aafc.h"
#include <iostream>

// The index table of ADPCM
constexpr int adpcm_index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
};

// The step table of ADPCM
constexpr int adpcm_step_size_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static signed char* encode_adpcm(float* ptr, int samplelength, size_t& audsize) {
    signed char* adpcm_base = (signed char*)malloc((samplelength / 2) * sizeof(signed char));
    signed char* adpcm = adpcm_base;
    const int* stptr = adpcm_step_size_table;
    const int* itbptr = adpcm_index_table;

    int index = 0;
    int step;
    int delta;
    int diff;
    int valpred = 0;
    int vpdiff;
    int bufferstep;
    int outputbuffer;
    int sign;

    step = *stptr;

    bufferstep = 1;

    for (int i = 0; i < samplelength; ptr++, i++) {
        short sample = static_cast<short>(Clamp(*ptr * 32767.0f, -32768.0f, 32767.0f));

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
        bufferstep = !bufferstep;
    }

    audsize = (samplelength * sizeof(signed char) / 2);
    return adpcm_base;
}

static void decode_adpcm(const unsigned char* input, float* output, int sampleCount) {
    const signed char* adpcm = reinterpret_cast<const signed char*>(input + sizeof(AAFC_HEADER));
    const int* stptr = adpcm_step_size_table;
    const int* itbptr = adpcm_index_table;

    int index = 0;
    int step;
    int delta;
    int valpred = 0;
    int vpdiff;
    int bufferstep;
    int inputbuffer = 0;
    int sign;

    step = *stptr;
    bufferstep = 0;

    for (int i = 0; i < sampleCount; i++) {
        if (bufferstep) {
            delta = inputbuffer & 0xf;
        }
        else {
            inputbuffer = *adpcm++;
            delta = (inputbuffer >> 4) & 0xf;
        }
        bufferstep = !bufferstep;

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

        *output++ = valpred * INT16_REC;
    }
}