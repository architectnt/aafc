/*

    Custom lower-percision floating point sample type
    

    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "sfpcm.h"

inline void* encode_sfpcm(float* ptr, unsigned int samplelength, size_t* audsize, unsigned char bps) {
    switch (bps) {
        case 8: {
            unsigned char* f8 = (unsigned char*)malloc(samplelength * sizeof(unsigned char));
            unsigned char* sptr = f8;
            for (unsigned int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = minifloat(clampf(*ptr * 127.0f, -128.0f, 127.0f));
            }

            *audsize = samplelength * sizeof(unsigned char);
            return f8;
        }
        case 16: {
            unsigned short* f16 = (unsigned short*)malloc(samplelength * sizeof(unsigned short));
            unsigned short* sptr = f16;
            for (unsigned int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = halfpercision(*ptr);
            }

            *audsize = samplelength * sizeof(unsigned short);
            return f16;
        }
        default: {
            printf("AAFC PCM: invalid bits per sample. (8, 16, and 32 valid)\n");
            return NULL;
        }
    }
}

inline void decode_sfpcm(const unsigned char* input, float* output, const unsigned int sampleCount, const unsigned char bps) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);

    switch (bps) {
        case 8: {
            const char* sptr = (const char*)smpraw;
            for (unsigned int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = dminif(*sptr) * INT8_REC;
            }
            break;
        }
        case 16: {
            const short* sptr = (const short*)smpraw;
            for (unsigned int i = 0; i < sampleCount; output++, sptr++, i++) {
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