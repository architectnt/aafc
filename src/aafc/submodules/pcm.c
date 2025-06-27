/*

    Standard PCM


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include "../aafc.h"
#include "pcm.h"
#include "../helpers.h"
#include "../common.h"

void* encode_pcm(float* ptr, const AAFC_HEADER* h, size_t* audsize) {
    switch (h->bps) {
        case 1: { // unlol-ing all of this will break everything
            *audsize = ((size_t)h->samplelength + 7) / 8;
            unsigned char* const stbs = (unsigned char*)malloc(*audsize);
            memset(stbs, 0, *audsize);

            for (unsigned int i = 0; i < h->samplelength; ptr++, i++)
                stbs[i >> 3] |= (*ptr > 0) << (i & 7);
            return stbs;
        }
        case 3: {
            *audsize = ((size_t)h->samplelength + 1) / 2;
            unsigned char* const stbs = (unsigned char*)malloc(*audsize);
            unsigned char* sptr = stbs;

            for (unsigned int i = 0; i < h->samplelength; ptr += 2, i += 2) {
                unsigned char smp1 = (unsigned char)round(CLAMP(*ptr * 3.0f, -4.0f, 3.0f)) & 0x07;
                unsigned char smp2 = ((i + 1 < h->samplelength)
                    ? (unsigned char)round(CLAMP(*(ptr + 1) * 3.0f, -4.0f, 3.0f))
                    : 0) & 0x07;
                *sptr++ = (smp1) | (smp2 << 3);
            }
            return stbs;
        }
        case 4: { // LOL
            *audsize = ((size_t)h->samplelength + 1) / 2;
            unsigned char* const stbs = (unsigned char*)malloc(*audsize);
            unsigned char* sptr = stbs;

            for (unsigned int i = 0; i < h->samplelength; ptr += 2, i += 2) {
                unsigned char smp1 = (unsigned char)round(CLAMP(*ptr * 7.0f, -8.0f, 7.0f)) & 0x0F;
                unsigned char smp2 = ((i + 1 < h->samplelength) 
                    ? (unsigned char)round(CLAMP(*(ptr + 1) * 7.0f, -8.0f, 7.0f)) 
                    : 0) & 0x0F;

                *sptr++ = (smp1) | (smp2 << 4);
            }
            return stbs;
        }
        case 8: {
            *audsize = h->samplelength;
            signed char* const stbs = (signed char*)malloc(h->samplelength);
            signed char* sptr = stbs;
            for (unsigned int i = 0; i < h->samplelength; ptr++, sptr++, i++) {
                *sptr = (signed char)round(CLAMP(*ptr * 127.0f, -128.0f, 127.0f));
            }
            return stbs;
        }
        case 10:{
            *audsize = ((size_t)(h->samplelength + 3) / 4) * 5;
            unsigned char* const stbs = (unsigned char*)malloc(*audsize);
            unsigned char* sptr = stbs;

            // i don't know what to say about this
            for (unsigned int i = 0; i < h->samplelength; i += 4) {
                int sample1 = (int)round(CLAMP(*(ptr + i) * 511.0f, -512.0f, 511.0f));
                int sample2 = (i + 1 < h->samplelength) ? (int)round(CLAMP(*(ptr + i + 1) * 511.0f, -512.0f, 511.0f)) : 0;
                int sample3 = (i + 2 < h->samplelength) ? (int)round(CLAMP(*(ptr + i + 2) * 511.0f, -512.0f, 511.0f)) : 0;
                int sample4 = (i + 3 < h->samplelength) ? (int)round(CLAMP(*(ptr + i + 3) * 511.0f, -512.0f, 511.0f)) : 0;

                *sptr++ = (sample1 & 0xFF);
                *sptr++ = ((sample1 >> 8) & 0x03) | ((sample2 & 0x3F) << 2);
                *sptr++ = ((sample2 >> 6) & 0x0F) | ((sample3 & 0x0F) << 4);
                *sptr++ = ((sample3 >> 4) & 0x3F) | ((sample4 & 0x03) << 6);
                *sptr++ = (sample4 >> 2);
            }
            return stbs;
        }
        case 12: {
            *audsize = ((size_t)(h->samplelength + 1) / 2) * 3;
            char* stbs = (char*)malloc(*audsize);
            char* sptr = stbs;

            for (unsigned int i = 0; i < h->samplelength; i += 2) {
                short s1 = (short)CLAMP(*(ptr + i) * 2047.0f, -2048.0f, 2047.0f) & 0xFFF;

                short s2 = ((i < h->samplelength) 
                    ? (short)CLAMP(*(ptr + i + 1) * 2047.0f, -2048.0f, 2047.0f) 
                    : 0) & 0xFFF;

                *sptr++ = (s1 & 0xFF);
                *sptr++ = ((s1 >> 8) & 0x0F) | ((s2 & 0x0F) << 4);
                *sptr++ = (s2 >> 4) & 0xFF;
            }
            return stbs;
        }
        case 16: {
            *audsize = h->samplelength * sizeof(short);
            short* const stbs = (short*)malloc(*audsize);
            short* sptr = stbs;
            for (unsigned int i = 0; i < h->samplelength; ptr++, sptr++, i++) {
                *sptr = (short)CLAMP(*ptr * 32767.0f, -32768.0f, 32767.0f);
            }
            return stbs;
        }
        case 24: {
            *audsize = (size_t)h->samplelength * 3;
            
            char* const stbs = (char*)malloc(*audsize);
            char* sptr = stbs;
            for (unsigned int i = 0; i < h->samplelength; ptr++, i++) {
                int s = (int)CLAMP(*ptr * 8388607.0f, -8388608.0f, 8388607.0f) & 0xFFFFFF;
                *sptr++ = (s & 0xFF);
                *sptr++ = ((s >> 8) & 0xFF);
                *sptr++ = ((s >> 16) & 0xFF);
            }
            return stbs;
        }
        case 32: {
            *audsize = h->samplelength * sizeof(float);
            float* stbs = (float*)malloc(*audsize);
            memcpy(stbs, ptr, *audsize);
            return stbs;
        }
        default: {
            printf("%s", "AAFC PCM: invalid bits per sample. (1, 3, 4, 8, 10, 12, 16, 24, and 32 valid)\n");
            return NULL;
        }
    }
}

void decode_pcm(const unsigned char* smpraw, float* output, const AAFC_HEADER* h) {
    switch (h->bps) {
        case 1: {
            float mixvol = 0.4; // save your hearing.
            for (unsigned int i = 0; i < h->samplelength; i++) {
                *output++ = ((*(smpraw + (i >> 3)) >> (i & 7)) & 1) ? mixvol : -mixvol;
            }
            break;
        }
        case 3: {
            signed char sp = 0, sl = 0;
            for (unsigned int i = 0; i < h->samplelength; smpraw++, i += 2) {
                sp = *smpraw & 0x07;
                if (sp > 3) sp -= 8;
                *output++ = sp * INT3_REC;
                sl = (*smpraw >> 3) & 0x07;
                if (sl > 3) sl -= 8;
                *output++ = sl * INT3_REC;
            }
            break;
        }
        case 4: {
            signed char sp = 0, sl = 0;
            for (unsigned int i = 0; i < h->samplelength; smpraw++, i += 2) {
                sp = *smpraw & 0x0F;
                if (sp > 7) sp -= 16;
                *output++ = sp * INT4_REC;
                sl = (*smpraw >> 4) & 0x0F;
                if (sl > 7) sl -= 16;
                *output++ = sl * INT4_REC;
            }
            break;
        }
        case 8: {
            for (const signed char* sptr = (const signed char*)smpraw, *n = sptr + h->samplelength; sptr < n; output++, sptr++) {
                *output = *sptr * INT8_REC;
            }
            break;
        }
        case 10: {
            for (unsigned int i = 0; i < h->samplelength; output += 4, i += 4) {
                // no comment. D:
                int sample1 = (*smpraw) | ((*(smpraw + 1) & 0x03) << 8);
                int sample2 = ((*(smpraw + 1) & 0xFC) >> 2) | ((*(smpraw + 2) & 0x0F) << 6);
                int sample3 = ((*(smpraw + 2) & 0xF0) >> 4) | ((*(smpraw + 3) & 0x3F) << 4);
                int sample4 = ((*(smpraw + 3) & 0xC0) >> 6) | (*(smpraw + 4) << 2);

                if (sample1 & 0x200) sample1 |= 0xFFFFFC00;
                if (sample2 & 0x200) sample2 |= 0xFFFFFC00;
                if (sample3 & 0x200) sample3 |= 0xFFFFFC00;
                if (sample4 & 0x200) sample4 |= 0xFFFFFC00;

                *output = sample1 * INT10_REC;
                if (i + 1 < h->samplelength) *(output + 1) = sample2 * INT10_REC;
                if (i + 2 < h->samplelength) *(output + 2) = sample3 * INT10_REC;
                if (i + 3 < h->samplelength) *(output + 3) = sample4 * INT10_REC;

                smpraw += 5;
            }
            break;
        }
        case 12: {
            for (unsigned int i = 0; i < h->samplelength; output += 2, i += 2) {
                int sample1 = (*smpraw | ((*(smpraw + 1) & 0x0F) << 8)) & 0xFFF;

                if (sample1 & 0x800) sample1 |= 0xFFFFF000;

                *output = sample1 * INT12_REC;
                if (i + 1 < h->samplelength) {
                    int sample2 = ((*(smpraw + 1) & 0xF0) >> 4) | ((*(smpraw + 2) << 4));
                    if (sample2 & 0x800) sample2 |= 0xFFFFF000;
                    *(output + 1) = sample2 * INT12_REC;
                }
                smpraw += 3;
            }
            break;
        }
        case 16: {
            for (const short* sptr = (const short*)smpraw, *n = sptr + h->samplelength; sptr < n; output++, sptr++) {
                *output = *sptr * INT16_REC;
            }
            break;
        }
        case 24: {
            for (unsigned int i = 0; i < h->samplelength; output++, i++) {
                int s24 = (int)((*(smpraw + 3 * i)) & 0xFF) | ((int)(*(smpraw + 3 * i + 1)) & 0xFF) << 8 | ((int)(*(smpraw + 3 * i + 2)) & 0xFF) << 16;
                if (s24 & 0x800000) {
                    s24 |= 0xFF000000;
                }
                *output = s24 * INT24_REC;
            }
            break;
        }
        case 32: {
            for (const float* sptr = (const float*)smpraw, *n = sptr + h->samplelength; sptr < n; output++, sptr++) {
                *output = *sptr;
            }
            break;
        }
        default: {
            printf("%s", "AAFC PCM IMPORT: invalid bits per sample\n");
            return;
        }
    }
}