/*

    Custom lower-percision floating point sample type
    

    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "sfpcm.h"

void* encode_sfpcm(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    switch (h->bps) {
        case 8: {
            unsigned char* stbs = (unsigned char*)malloc(h->samplelength);
            unsigned char* sptr = stbs;
            for (unsigned int i = 0; i < h->samplelength; ptr++, sptr++, i++) {
                *sptr = minifloat(clampf(*ptr * 127.0f, -128.0f, 127.0f));
            }

            *audsize = h->samplelength;
            return stbs;
        }
        case 16: {
            size_t bsize = h->samplelength * sizeof(short);
            unsigned short* stbs = (unsigned short*)malloc(bsize);
            unsigned short* sptr = stbs;
            for (unsigned int i = 0; i < h->samplelength; ptr++, sptr++, i++) {
                *sptr = halfpercision(*ptr);
            }

            *audsize = bsize;
            return stbs;
        }
        default: {
            printf("AAFC PCM: invalid bits per sample. (8, 16, and 32 valid)\n");
            return NULL;
        }
    }
}

void decode_sfpcm(const unsigned char* input, float* output, const AAFC_HEADER* h) {
    switch (h->bps) {
        case 8: {
            const char* sptr = (const char*)input;
            for (unsigned int i = 0; i < h->samplelength; output++, sptr++, i++) {
                *output = dminif(*sptr) * INT8_REC;
            }
            break;
        }
        case 16: {
            const short* sptr = (const short*)input;
            for (unsigned int i = 0; i < h->samplelength; output++, sptr++, i++) {
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