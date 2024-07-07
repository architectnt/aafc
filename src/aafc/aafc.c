/*

    Main AAFC Module
    Contains everything the format has to offer


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "lib.h"

EXPORT AAFC_HEADER* aafc_getheader(const unsigned char* bytes) {
    if (header_valid(bytes)) {
        return (AAFC_HEADER*)bytes;
    }
    else {
        return NULL;
    }
}

EXPORT AAFCOUTPUT aafc_export(float* samples, int freq, unsigned char channels, int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, int samplerateoverride, bool nm, float pitch) {
    if (!samples || bps == 0 || sampletype == 0) {
        printf("AAFC FATAL ERROR: samples, bps or sample type not set\n");
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }

    if (pitch == 0) {
        pitch = 1;
    }

    AAFC_HEADER* header = (AAFC_HEADER*)malloc(sizeof(AAFC_HEADER));
    if (!header) {
        printf("AAFC FATAL ERROR: could not allocate a new header.\n");
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }

    if (!create_header(header, freq, channels, samplelength, bps, sampletype)) {
        printf("AAFC FATAL ERROR: could not create header\n");
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }

    float* rsptr = samples;

    if (forcemono && channels != 1) {
        forceMono(rsptr, header, &channels, &samplelength);
    }

    if (samplerateoverride != 0 && samplerateoverride != freq || pitch != 1) {
        rsptr = resampleAudio(rsptr, header, samplerateoverride, freq, channels, &samplelength, pitch);
    }

    if (nm) {
        normalize(rsptr, samplelength);
    }

    void* smpl = NULL;
    size_t audioDataSize = 0;

    if (samplelength > 1) {
        switch (sampletype) {
            case 1: {
                smpl = encode_pcm(rsptr, samplelength, &audioDataSize, bps);
                break;
            }
            case 2: {
                if (channels > 1) {
                    rsptr = force_independent_channels(rsptr, channels, samplelength);
                }
                smpl = encode_adpcm(rsptr, samplelength, &audioDataSize);
                break;
            }
            case 3: {
                smpl = encode_dpcm(rsptr, samplelength, &audioDataSize);
                break;
            }
            case 4: {
                smpl = encode_sfpcm(rsptr, samplelength, &audioDataSize, bps);
                break;
            }
            case 5: {
                smpl = encode_ulaw(rsptr, samplelength, &audioDataSize);
                break;
            }
            default: {
                free(header);
                free(rsptr);
                printf("AAFC ERROR: Invalid sample type!\n");
                AAFCOUTPUT output = { NULL, 0 };
                return output;
            }
        }
    }
    else {
        free(header);
        free(rsptr);
        printf("AAFC ERROR: samplelength cannot be below 1.\n");
        AAFCOUTPUT output = { NULL, 0 };
        return output;
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
        AAFCOUTPUT output = { rst, totalDataSize };
        return output;
    }
    else {
        free(rsptr);
        free(header);
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }
}

EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes) {
    if (header_valid(bytes)) {
        AAFC_HEADER* header = (AAFC_HEADER*)bytes;
        int sampleCount = header->samplelength;
        int bps = header->bps;
        int sampletype = header->sampletype;

        AAFCDECOUTPUT output = { *header };

        output.data = (float*)malloc(sampleCount * sizeof(float));
        if (output.data == NULL) {
            printf("AAFC: FAILED ALLOCATION OF DATA\n");
            AAFCDECOUTPUT noutp = { };
            return noutp;
        }
        float* rsptr = output.data;

        switch (sampletype) {
            case 1: {
                decode_pcm(bytes, rsptr, sampleCount, bps);
                break;
            }
            case 2: {
                decode_adpcm(bytes, rsptr, sampleCount);
                if (header->channels > 1) {
                    float* itrsamples = force_interleave_channels(output.data, header->channels, sampleCount);
                    if (itrsamples) {
                        free(output.data);
                        output.data = itrsamples;
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
            case 5: {
                decode_ulaw(bytes, rsptr, sampleCount);
                break;
            }
            default: {
                free(output.data);
                printf("AAFC ERROR: Invalid sample type!\n");
                AAFCDECOUTPUT noutp = { };
                return noutp;
            }
        }

        if (output.data) {
            return output;
        }
        else {
            AAFCDECOUTPUT noutp = { };
            return noutp;
        }
    }
    else {
        printf("AAFC: invalid aafc data\n");
        AAFCDECOUTPUT noutp = { };
        return noutp;
    }
}

EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end)
{
    float* samples = (float*)malloc(end * sizeof(float));
    if (header_valid(bytes)) {
        AAFC_HEADER* header = (AAFC_HEADER*)bytes;
        int sampleCount = header->samplelength;
        int bps = header->bps;
        const unsigned char* smpraw = bytes + sizeof(AAFC_HEADER);

        //TODO: support more sample types

        if (bps == 8) {
            const char* sptr = (const char*)smpraw;
            for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                samples[i] = sptr[i] * INT8_REC;
            }
        }
        else if (bps == 16) {
            const short* sptr = (const short*)smpraw;
            for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                samples[i] = sptr[i] * INT16_REC;
            }
        }
        else if (bps == 32) {
            const float* inputf = (const float*)smpraw;
            for (int i = start; i < end && i >= 0 && i <= sampleCount; i++) {
                samples[i] = inputf[i];
            }
        }
        else {
            free(samples);
            printf("AAFC PCM IMPORT: invalid bits per sample\n");
            return NULL;
        }
    }
    else {
        printf("AAFC does not support chunk reading for AAFC versions before AAFC v2.");
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
                *sptr = (char)round(Clamp(*aptr * 127.0f, -128.0f, 127.0f));
            }
            rst = csmpl;
            break;
        }
        case 16: {
            short* csmpl = (short*)malloc(size * sizeof(short));
            short* sptr = csmpl;
            for (int i = 0; i < size; aptr++, sptr++, i++) {
                *sptr = (short)Clamp(*aptr * 32767.0f, -32768.0f, 32767.0f);
            }
            rst = csmpl;
            break;
        }
        case 32: {
            int* csmpl = (int*)malloc(size * sizeof(int));
            int* sptr = csmpl;
            for (int i = 0; i < size; aptr++, sptr++, i++) {
                *sptr = (short)Clamp(*aptr * 2147483647.0f, -2147483648.0f, 2147483647.0f);
            }
            rst = csmpl;
            break;
        }
        default: {
            printf("unknown integer type: %d", type);
            return NULL;
        }
    }

    return rst;
}

// compatibility layer for systems that use integers instead (useful for exporting)
EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type) {
    float* csmpl = (float*)malloc(size * sizeof(float));
    switch (type) {
        case 8: {
            char* sptr = (char*)arr;
            for (int i = 0; i < size; i++) {
                *(csmpl + i) = *(sptr + i) * INT8_REC;
            }
            break;
        }
        case 16: {
            short* sptr = (short*)arr;
            for (int i = 0; i < size; i++) {
                *(csmpl + i) = *(sptr + i) * INT16_REC;
            }
            break;
        }
        case 32: {
            int* sptr = (int*)arr;
            for (int i = 0; i < size; i++) {
                *(csmpl + i) = *(sptr + i) * INT32_REC;
            }
            break;
        }
        default: {
           free(csmpl);
           printf("unknown integer type: %d", type);
            return NULL;
        }
    }

    return csmpl;
}

EXPORT float* aafc_resample_data(float* input, int samplerateoverride, int freq, unsigned char channels, int samplelength, float pitch) {
    return resampleAudio(input, NULL, samplerateoverride, freq, channels, &samplelength, pitch);
}

EXPORT float* aafc_normalize(float* arr, int len) {
    return normalize(arr, len);
}

#if 0

EXPORT AAFCFILETABLE aft_create(unsigned char*** data, size_t tablelength, size_t* sizes) {
    AAFCFILETABLE ftable = {};

    // TODO: REWORK FILE TABLES

    return ftable;
}

EXPORT AAFCOUTPUT aft_export(AAFCFILETABLE* ftable) {
    return create_filetable_stream(ftable);
}

EXPORT AAFCFILETABLE* aft_import(unsigned char* data) {
    return decode_filetable_stream(data);
}

EXPORT AAFCOUTPUT aft_get_clip(AAFCFILETABLE* ftable, unsigned char group, unsigned short index) {
    if (group >= ftable->size) {
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }

    FILETABLE* filetable = &ftable->filetables[group];
    if (index >= filetable->size) {
        AAFCOUTPUT output = { NULL, 0 };
        return output;
    }

    DATATABLE* datatable = &filetable->data[index];

    AAFCOUTPUT output = { datatable->data, datatable->len };
    return output;
}

#endif