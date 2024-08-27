/*

    Standard PCM


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "pcm.h"

void* encode_pcm(float* ptr, unsigned int samplelength, size_t* audsize, unsigned char bps) {
    switch (bps) {
        case 1: { // unlol-ing all of this will break everything
            printf(":>\n");
            size_t bsize = ((size_t)samplelength + 7) / 8;

            unsigned char* stbs = (unsigned char*)malloc(bsize);
            memset(stbs, 0, bsize);

            for (int i = 0; i < samplelength; ptr++, i++)
                *(stbs + (i >> 3)) |= (*ptr > 0) << (i & 7);

            *audsize = bsize;
            return stbs;
        }
        case 4: { // LOL
            printf(";)\n");
            size_t bsize = ((size_t)samplelength + 1) / 2;
            unsigned char* stbs = (unsigned char*)malloc(bsize);
            unsigned char* sptr = stbs;

            for (int i = 0; i < samplelength; ptr += 2, i += 2) {
                int smp1 = (int)round(clampf(*ptr * 7.0f, -8.0f, 7.0f));
                smp1 = smp1 < 0 ? smp1 + 16 : smp1;

                int smp2 = 0;
                if (i + 1 < samplelength) {
                    smp2 = (int)round(clampf(*(ptr + 1) * 7.0f, -8.0f, 7.0f));
                    smp2 = smp2 < 0 ? smp2 + 16 : smp2;
                }

                *sptr++ = (smp1 & 0x0F) | (smp2 << 4);
            }
            *audsize = bsize;
            return stbs;
        }
        case 8: {
            char* stbs = (char*)malloc(samplelength);
            char* sptr = stbs;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = (char)round(clampf(*ptr * 127.0f, -128.0f, 127.0f));
            }
            *audsize = samplelength;
            return stbs;
        }
        case 10:{
            size_t bsize = ((size_t)(samplelength + 3) / 4) * 5;
            unsigned char* stbs = (unsigned char*)malloc(bsize);
            unsigned char* sptr = stbs;

            // i don't know what to say about this
            for (int i = 0; i < samplelength; i += 4) {
                int sample1 = (int)round(clampf(*(ptr + i) * 511.0f, -512.0f, 511.0f));
                int sample2 = (i + 1 < samplelength) ? (int)round(clampf(*(ptr + i + 1) * 511.0f, -512.0f, 511.0f)) : 0;
                int sample3 = (i + 2 < samplelength) ? (int)round(clampf(*(ptr + i + 2) * 511.0f, -512.0f, 511.0f)) : 0;
                int sample4 = (i + 3 < samplelength) ? (int)round(clampf(*(ptr + i + 3) * 511.0f, -512.0f, 511.0f)) : 0;

                *sptr++ = (sample1 & 0xFF);
                *sptr++ = ((sample1 >> 8) & 0x03) | ((sample2 & 0x3F) << 2);
                *sptr++ = ((sample2 >> 6) & 0x0F) | ((sample3 & 0x0F) << 4);
                *sptr++ = ((sample3 >> 4) & 0x3F) | ((sample4 & 0x03) << 6);
                *sptr++ = (sample4 >> 2);
            }

            *audsize = bsize;
            return stbs;
        }
        case 12: {
            size_t bsize = ((size_t)(samplelength + 1) / 2) * 3;
            char* stbs = (char*)malloc(bsize);
            char* sptr = stbs;

            for (int i = 0; i < samplelength; i += 2) {
                int sample1 = (int)clampf(*(ptr + i) * 2047.0f, -2048.0f, 2047.0f);
                if (sample1 < 0) sample1 = 0xFFF + sample1 + 1;


                int sample2 = (i < samplelength) ? (int)clampf(*(ptr + i + 1) * 2047.0f, -2048.0f, 2047.0f) : 0;
                if (sample2 < 0) sample1 = 0xFFF + sample1 + 1;

                *sptr++ = (sample1 & 0xFF);
                *sptr++ = ((sample1 >> 8) & 0x0F) | ((sample2 & 0x0F) << 4);
                *sptr++ = (sample2 >> 4);
            }

            *audsize = bsize;
            return stbs;
        }
        case 16: {
            size_t bsize = samplelength * sizeof(short);

            short* stbs = (short*)malloc(bsize);
            short* sptr = stbs;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = (short)clampf(*ptr * 32767.0f, -32768.0f, 32767.0f);
            }
            *audsize = bsize;
            return stbs;
        }
        case 24: {
            size_t bsize = (size_t)samplelength * 3;

            char* stbs = (char*)malloc(bsize);
            char* sptr = stbs;
            for (int i = 0; i < samplelength; ptr++, i++) {
                int spl24 = (int)clampf(*ptr * 8388607.0f, -8388608.0f, 8388607.0f);
    
                if (spl24 < 0) {
                   spl24 = 0xFFFFFF + spl24 + 1;
                }

                *sptr++ = (spl24 & 0xFF);
                *sptr++ = ((spl24 >> 8) & 0xFF);
                *sptr++ = ((spl24 >> 16) & 0xFF);
            }
            *audsize = bsize;
            return stbs;
        }
        case 32: {
            size_t bsize = samplelength * sizeof(float);

            float* stbs = (float*)malloc(bsize);
            memcpy(stbs, ptr, bsize);
            *audsize = bsize;
            return stbs;
        }
        default: {
            printf("AAFC PCM: invalid bits per sample. (1, 4, 8, 10, 12, 16, 24, and 32 valid)\n");
            return NULL;
        }
    }
}

void decode_pcm(const unsigned char* input, float* output, const unsigned int sampleCount, const unsigned char bps) {
    const unsigned char* smpraw = input + sizeof(AAFC_HEADER);

    switch (bps) {
        case 1: {
            printf("L O L\n");
            float mixvol = 0.4;
            for (unsigned int i = 0; i < sampleCount; i++) {
                *output++ = ((*(smpraw + (i >> 3)) >> (i & 7)) & 1) ? mixvol : -mixvol;
            }
            break;
        }
        case 4: {
            printf("LOL\n");
            int sp = 0, sl = 0;
            for (unsigned int i = 0; i < sampleCount; smpraw++, i += 2) {
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
            for (const char* sptr = (const char*)smpraw, *n = sptr + sampleCount; sptr < n; output++, sptr++) {
                *output = *sptr * INT8_REC;
            }
            break;
        }
        case 10: {
            for (unsigned int i = 0; i < sampleCount; output += 4, i += 4) {
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
                if (i + 1 < sampleCount) *(output + 1) = sample2 * INT10_REC;
                if (i + 2 < sampleCount) *(output + 2) = sample3 * INT10_REC;
                if (i + 3 < sampleCount) *(output + 3) = sample4 * INT10_REC;

                smpraw += 5;
            }
            break;
        }
        case 12: {
            for (unsigned int i = 0; i < sampleCount; output += 2, i += 2) {
                int sample1 = (*smpraw | ((*(smpraw + 1) & 0x0F) << 8)) & 0xFFF;

                if (sample1 & 0x800) sample1 |= 0xFFFFF000;

                *output = sample1 * INT12_REC;
                if (i + 1 < sampleCount) {
                    int sample2 = ((*(smpraw + 1) & 0xF0) >> 4) | ((*(smpraw + 2) << 4));
                    if (sample2 & 0x800) sample2 |= 0xFFFFF000;
                    *(output + 1) = sample2 * INT12_REC;
                }
                smpraw += 3;
            }
            break;
        }
        case 16: {
            for (const short* sptr = (const short*)smpraw, *n = sptr + sampleCount; sptr < n; output++, sptr++) {
                *output = *sptr * INT16_REC;
            }
            break;
        }
        case 24: {
            for (unsigned int i = 0; i < sampleCount; output++, i++) {
                int s24 = (int)((*(smpraw + 3 * i)) & 0xFF) | ((int)(*(smpraw + 3 * i + 1)) & 0xFF) << 8 | ((int)(*(smpraw + 3 * i + 2)) & 0xFF) << 16;
                if (s24 & 0x800000) {
                    s24 |= 0xFF000000;
                }
                *output = s24 * INT24_REC;
            }
            break;
        }
        case 32: {
            for (const float* sptr = (const float*)smpraw, *n = sptr + sampleCount; sptr < n; output++, sptr++) {
                *output = *sptr;
            }
            break;
        }
        default: {
            printf("AAFC PCM IMPORT: invalid bits per sample\n");
            return;
        }
    }
}