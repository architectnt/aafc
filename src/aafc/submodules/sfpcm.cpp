/*

    Custom lower-percision floating point sample type
    

    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#include <iostream>
#include "helpers.h"
#include "aafc.h"
#include "common.h"

static void* encode_sfpcm(float* ptr, int samplelength, size_t& audsize, unsigned char bps) {
    switch (bps) {
        case 8: {
            unsigned char* f8 = (unsigned char*)malloc(samplelength * sizeof(unsigned char));
            unsigned char* sptr = f8;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = minifloat(Clamp(*ptr * 127.0f, -128.0f, 127.0f));
            }

            audsize = samplelength * sizeof(unsigned char);
            return f8;
        }
        case 16: {
            unsigned short* f16 = (unsigned short*)malloc(samplelength * sizeof(unsigned short));
            unsigned short* sptr = f16;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = halfpercision(*ptr);
            }

            audsize = samplelength * sizeof(unsigned short);
            return f16;
        }
        default: {
            printf("AAFC PCM: invalid bits per sample. (8, 16, and 32 valid)\n");
            return nullptr;
        }
    }
}

static void decode_sfpcm(const unsigned char* input, float* output, int sampleCount, unsigned char bps) {
    switch (bps) {
        case 8: {
            const char* smpraw = reinterpret_cast<const char*>(input + sizeof(AAFC_HEADER));
            const char* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = dminif(*sptr) * INT8_REC;
            }
            break;
        }
        case 16: {
            const short* smpraw = reinterpret_cast<const short*>(input + sizeof(AAFC_HEADER));
            const short* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = dhalf(*sptr);
            }
            break;
        }
        default: {
            printf("AAFC PCM IMPORT: invalid bits per sample\n");
            return;
        }
    }
}