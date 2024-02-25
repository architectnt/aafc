/*

    Standard PCM
    2023-2024 Architect Enterprises

*/

#include <iostream>
#include "helpers.h"
#include "aafc.h"

static void* encode_pcm(float* ptr, int samplelength, size_t& audsize, unsigned char bps) {
    switch (bps) {
        case 8: {
            char* stbs8 = (char*)malloc(samplelength * sizeof(char));
            char* sptr = stbs8;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = static_cast<char>(round(Clamp(*ptr * 127.0f, -128.0f, 127.0f)));
            }
            audsize = samplelength * sizeof(unsigned char);
            return stbs8;
        }case 12: {
            size_t bsize = (samplelength * 3) / 2;
            char* stbs12 = (char*)malloc(bsize * sizeof(char));
            char* sptr = stbs12;

            for (int i = 0; i < samplelength; ptr += 2, i += 2) {
                int sample1 = static_cast<int>(Clamp(*ptr * 2047.0f, -2048.0f, 2047.0f));
                if (sample1 < 0) {
                    sample1 = 0xFFF + sample1 + 1;
                }

                int sample2 = 0;
                if (i + 1 < samplelength) {
                    sample2 = static_cast<int>(Clamp(*(ptr + 1) * 2047.0f, -2048.0f, 2047.0f));
                    if (sample2 < 0) {
                        sample2 = 0xFFF + sample2 + 1;
                    }
                }
                *sptr++ = (sample1 & 0xFF);
                *sptr++ = ((sample1 >> 8) & 0x0F) | ((sample2 & 0x0F) << 4);
                *sptr++ = (sample2 >> 4);
            }

            audsize = bsize;
            return stbs12;
        }case 16: {
            short* stbs16 = (short*)malloc(samplelength * sizeof(short));
            short* sptr = stbs16;
            for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
                *sptr = static_cast<short>(Clamp(*ptr * 32767.0f, -32768.0f, 32767.0f));
            }
            audsize = samplelength * sizeof(short);
            return stbs16;
        }case 24: {
            char* stbs24 = (char*)malloc(samplelength * 3 * sizeof(char));
            char* sptr = stbs24;
            for (int i = 0; i < samplelength; ptr++, i++) {
                int spl24 = static_cast<int>(Clamp(*ptr * 8388607.0f, -8388608.0f, 8388607.0f));
    
                if (spl24 < 0) {
                   spl24 = 0xFFFFFF + spl24 + 1;
                }

                *sptr++ = (spl24 & 0xFF);
                *sptr++ = ((spl24 >> 8) & 0xFF);
                *sptr++ = ((spl24 >> 16) & 0xFF);
            }
            audsize = samplelength * 3 * sizeof(char);
            return stbs24;
        }case 32: {
            float* stbsf = (float*)malloc(samplelength * sizeof(float));
            memcpy(stbsf, ptr, samplelength * sizeof(float));
            audsize = samplelength * sizeof(float);
            return stbsf;
        }default: {
            printf("AAFC PCM: invalid bits per sample. (8, 16, and 32 valid)\n");
            return nullptr;
        }
    }
}

static void decode_pcm(const unsigned char* input, float* output, int sampleCount, unsigned char bps) {
    switch (bps) {
        case 8: {
            const char* smpraw = reinterpret_cast<const char*>(input + sizeof(AAFC_HEADER));
            const char* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = *sptr * INT8_REC;
            }
            break;
        }
        case 12: {
            const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
            for (int i = 0; i < sampleCount; output += 2, i += 2) {
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
            const short* smpraw = reinterpret_cast<const short*>(input + sizeof(AAFC_HEADER));
            const short* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = *sptr * INT16_REC;
            }
            break;
        }
        case 24: {
            const unsigned char* smpraw = input + sizeof(AAFC_HEADER);
            for (int i = 0; i < sampleCount; output++, i++) {
                int s24 = (int)(*(smpraw + 3 * i)) & 0xFF | ((int)(*(smpraw + 3 * i + 1)) & 0xFF) << 8 | ((int)(*(smpraw + 3 * i + 2)) & 0xFF) << 16;
                if (s24 & 0x800000) {
                    s24 |= 0xFF000000;
                }
                *output = s24 * INT24_REC;
            }
            break;
        }
        case 32: {
            const float* inputf = reinterpret_cast<const float*>(input + sizeof(AAFC_HEADER));
            for (int i = 0; i < sampleCount; output++, i++) {
                *output = *(inputf + i);
            }
            break;
        }
        default: {
            printf("AAFC PCM IMPORT: invalid bits per sample\n");
            return;
        }
    }
}