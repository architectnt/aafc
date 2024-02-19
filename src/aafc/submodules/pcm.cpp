/*

    Standard PCM
    2023-2024 Architect Enterprises

*/

#include <iostream>
#include "helpers.h"
#include "aafc.h"

static void* encode_pcm(float* ptr, int samplelength, size_t& audsize, unsigned char bps) {
    if (bps == 8) {
        char* stbs8 = (char*)malloc(samplelength * sizeof(char));
        char* sptr = stbs8;
        for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
            *sptr = static_cast<char>(round(Clamp(*ptr * 127.0f, -128.0f, 127.0f)));
        }
        audsize = samplelength * sizeof(unsigned char);
        return stbs8;
    }
    else if (bps == 16) {
        short* stbs16 = (short*)malloc(samplelength * sizeof(short));
        short* sptr = stbs16;
        for (int i = 0; i < samplelength; ptr++, sptr++, i++) {
            *sptr = static_cast<short>(Clamp(*ptr * 32767.0f, -32768.0f, 32767.0f));
        }
        audsize = samplelength * sizeof(short);
        return stbs16;
    }
    else if (bps == 24)
    {
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
    }
    else if (bps == 32) {
        float* stbsf = (float*)malloc(samplelength * sizeof(float));
        memcpy(stbsf, ptr, samplelength * sizeof(float));
        audsize = samplelength * sizeof(float);
        return stbsf;
    }
    else {
        printf("AAFC PCM: invalid bits per sample. (8, 16, and 32 valid)\n");
        return nullptr;
    }
}

static void decode_pcm(const unsigned char* input, float* output, int sampleCount, unsigned char bps) {
    switch (bps) {
        case 8: {
            char* smpraw = reinterpret_cast<char*>(const_cast<unsigned char*>(input + sizeof(AAFC_HEADER)));
            char* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = *sptr * INT8_REC;
            }
            break;
        }
        case 16: {
            short* smpraw = reinterpret_cast<short*>(const_cast<unsigned char*>(input + sizeof(AAFC_HEADER)));
            short* sptr = smpraw;
            for (int i = 0; i < sampleCount; output++, sptr++, i++) {
                *output = *sptr * INT16_REC;
            }
            break;
        }
        case 24: {
            char* smpraw = reinterpret_cast<char*>(const_cast<unsigned char*>(input + sizeof(AAFC_HEADER)));
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
            float* inputf = reinterpret_cast<float*>(const_cast<unsigned char*>(input + sizeof(AAFC_HEADER)));
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