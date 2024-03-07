/*

    Main AAFC Module
    Contains everything the format has to offer


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <iostream>

#include "aafc.h"
#include "common.h"
#include "submodules/misc.cpp"
#include "submodules/modifiers.cpp"
#include "submodules/adpcm.cpp"
#include "submodules/dpcm.cpp"
#include "submodules/pcm.cpp"
#include "submodules/sfpcm.cpp"

extern "C" {
    EXPORT void* aafc_getheader(const unsigned char* bytes) {
        if (header_valid(bytes)) {
            return reinterpret_cast<AAFC_HEADER*>(const_cast<unsigned char*>(bytes));
        }
        else {
            return reinterpret_cast<int*>(const_cast<unsigned char*>(bytes));
        }
    }

    EXPORT unsigned char* aafc_export(float* samples, int freq, unsigned char channels, int samplelength, unsigned char bps = 16, unsigned char sampletype = 1, bool forcemono = false, int samplerateoverride = 0) {
        if (!samples) {
            return nullptr;
        }

        AAFC_HEADER* header = (AAFC_HEADER*)malloc(sizeof(AAFC_HEADER));
        if (!header) {
            printf("AAFC FATAL ERROR: could not allocate a new header.\n");
            return nullptr;
        }
        
        if (!create_header(header, freq, channels, samplelength, bps, sampletype)) {
            printf("AAFC FATAL ERROR: could not create header\n");
            return nullptr;
        }

        float* rsptr = samples;

        if (forcemono && channels != 1) {
            forceMono(samples, header, channels, samplelength);
        }
        if (samplerateoverride != 0 && samplerateoverride != freq) {
            rsptr = resampleAudio(rsptr, header, samplerateoverride, freq, channels, samplelength);
        }

        void* smpl = nullptr;
        size_t audioDataSize = 0;

        if (samplelength > 1) {
            switch (sampletype) {
                case 1: {
                    smpl = encode_pcm(rsptr, samplelength, audioDataSize, bps);
                    break;
                }
                case 2: {
                    if (channels > 1) {
                        rsptr = force_independent_channels(rsptr, channels, samplelength);
                    }

                    smpl = encode_adpcm(rsptr, samplelength, audioDataSize);
                    break;
                }
                case 3: {
                    smpl = encode_dpcm(rsptr, samplelength, audioDataSize);
                    break;
                }
                case 4: {
                    smpl = encode_sfpcm(rsptr, samplelength, audioDataSize, bps);
                    break;
                }
                default: {
                    free(header);
                    free(rsptr);
                    printf("AAFC ERROR: Invalid sample type!\n");
                    return nullptr;
                }
            }
        }
        else {
            free(header);
            free(rsptr);
            printf("AAFC ERROR: samplelength cannot be below 1.\n");
            return nullptr;
        }

        size_t totalDataSize = sizeof(AAFC_HEADER) + audioDataSize;
        if (smpl) {
            unsigned char* rst = (unsigned char*)malloc(totalDataSize);

            memcpy(rst, header, sizeof(AAFC_HEADER));
            memcpy(rst + sizeof(AAFC_HEADER), smpl, audioDataSize);
            freeSamples(smpl, bps, sampletype);

            free(header);
            if (rsptr != samples) {
                free(rsptr);
            }

            return rst;
        }
        else {
            free(rsptr);
            free(header);
            return NULL;
        }
    }

    EXPORT float* aafc_import(const unsigned char* bytes) {
        int sampleCount = 0;
        if (header_valid(bytes)) {
            AAFC_HEADER* header = reinterpret_cast<AAFC_HEADER*>(const_cast<unsigned char*>(bytes));
            sampleCount = header->samplelength;
            int bps = header->bps;
            int sampletype = header->sampletype;

            float* samples = (float*)malloc(sampleCount * sizeof(float));
            float* rsptr = samples;

            switch (sampletype) {
                case 1: {
                    decode_pcm(bytes, rsptr, sampleCount, bps);
                    break;
                }
                case 2: {
                    decode_adpcm(bytes, rsptr, sampleCount);
                    if (header->channels > 1) {
                        float* itrsamples = force_interleave_channels(rsptr, header->channels, sampleCount);
                        if (itrsamples) {
                            free(samples);
                            samples = itrsamples;
                        }
                    }
                    break;
                }
                case 3: {
                    decode_dpcm(bytes, rsptr, sampleCount);
                    break;
                }
                case 4: {
                    decode_sfpcm(bytes, rsptr, sampleCount, bps);
                    break;
                }
                default: {
                    free(samples);
                    printf("AAFC ERROR: Invalid sample type!\n");
                    return nullptr;
                }
            }

            if (rsptr || samples) {
                return samples;
            }
            else {
                return nullptr;
            }
        }
        else {
            //TODO: deprecate this fallback format if there is no such aafc1 file in system lollllllll
            int* iptr = reinterpret_cast<int*>(const_cast<unsigned char*>(bytes));
            sampleCount = *(iptr + 2);
            short* smpraw = reinterpret_cast<short*>(const_cast<unsigned char*>(bytes + 12));
            float* samples = (float*)malloc(sampleCount);

            float* rsptr = samples;
            short* sptr = smpraw;

            for (int i = 0; i < sampleCount; rsptr++, sptr++, i++) {
                *rsptr = *sptr * INT16_REC;
            }

            return samples;
        }
    }

    EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end)
    {
        float* samples = (float*)malloc(end * sizeof(float));
        if (header_valid(bytes)) {
            AAFC_HEADER* header = reinterpret_cast<AAFC_HEADER*>(const_cast<unsigned char*>(bytes));
            int sampleCount = header->samplelength;
            int bps = header->bps;
            const int boffst = sizeof(AAFC_HEADER);

            //TODO: support more sample types

            if (bps == 8) {
                char* smpraw = reinterpret_cast<char*>(const_cast<unsigned char*>(bytes + boffst));
                for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                    samples[i] = smpraw[i] * INT8_REC;
                }
            }
            else if (bps == 16) {
                short* smpraw = reinterpret_cast<short*>(const_cast<unsigned char*>(bytes + boffst));
                for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                    samples[i] = smpraw[i] * INT16_REC;
                }
            }
            else if (bps == 32) {
                float* tmpsamples = reinterpret_cast<float*>(const_cast<unsigned char*>(bytes + boffst));
                for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                    samples[i] = tmpsamples[i];
                }
            }
            else {
                free(samples);
                printf("AAFC PCM IMPORT: invalid bits per sample\n");
                return nullptr;
            }
        }
        else {
            printf("AAFC2 does not support chunk reading for older AAFC versions.");
            for (int i = 0; i < end; i++)
            {
                samples[i] = 0;
            }
        }
        return samples;
    }

    // compatibility layer for any subsystem that can only use integers instead of floats
    EXPORT void* aafc_float_to_int(float* arr, long size, unsigned char type) {
        void* rst;
        float* aptr = arr;

        switch (type) {
            case 8: {
                char* csmpl = (char*)malloc(size * sizeof(char));
                char* sptr = csmpl;
                for (int i = 0; i < size; aptr++, sptr++, i++) {
                    *sptr = static_cast<char>(round(Clamp(*aptr * 127.0f, -128.0f, 127.0f)));
                }
                rst = csmpl;
                break;
            }
            case 16: {
                short* csmpl = (short*)malloc(size * sizeof(short));
                short* sptr = csmpl;
                for (int i = 0; i < size; aptr++, sptr++, i++) {
                    *sptr = static_cast<short>(Clamp(*aptr * 32767.0f, -32768.0f, 32767.0f));
                }
                rst = csmpl;
                break;
            }
            case 32: {
                int* csmpl = (int*)malloc(size * sizeof(int));
                int* sptr = csmpl;
                for (int i = 0; i < size; aptr++, sptr++, i++) {
                    *sptr = static_cast<int>(Clamp(*aptr * 2147483647.0f, -2147483648.0f, 2147483647.0f));
                }
                rst = csmpl;
                break;
            }
            default: {
                printf("unknown integer type: %d", type);
                return nullptr;
            }
        }

        return rst;
    }

    // compatibility layer for systems that use integers instead (useful for exporting)
    EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type) {
        float* rst;
        float* csmpl = (float*)malloc(size * sizeof(float));
        switch (type) {
            case 8: {
                char* sptr = (char*)arr;
                for (int i = 0; i < size; i++) {
                    *(csmpl + i) = *(sptr + i) * INT8_REC;
                }
                rst = csmpl;
                break;
            }
            case 16: {
                short* sptr = (short*)arr;
                for (int i = 0; i < size; i++) {
                    *(csmpl + i) = *(sptr + i) * INT16_REC;
                }
                rst = csmpl;
                break;
            }
            case 32: {
                int* sptr = (int*)arr;
                for (int i = 0; i < size; i++) {
                    *(csmpl + i) = *(sptr + i) * INT32_REC;
                }
                rst = csmpl;
                break;
            }
            default: {
                free(csmpl);
                printf("unknown integer type: %d", type);
                return nullptr;
            }
        }

        return rst;
    }

    EXPORT float* aafc_resample_data(float* input, int samplerateoverride, int freq, unsigned char channels, int& samplelength) {
        return resampleAudio(input, nullptr, samplerateoverride, freq, channels, samplelength);
    }

    EXPORT void aafc_free(float* arr) {
        if (arr != nullptr) {
            free(arr);
        }
        else {
            printf("AAFC: the sample data is null!");
        }
    }

    EXPORT void aafc_free_bytes(unsigned char* arr) {
        if (arr != nullptr) {
            free(arr);
        }
        else {
            printf("AAFC: the input bytes are null!");
        }
    }
}