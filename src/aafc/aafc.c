/*

    Main AAFC Module
    Contains everything the format has to offer


    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "lib.h"

EXPORT unsigned short aafc_getversion() {
    return AAFCVERSION;
}

EXPORT AAFC_HEADER* aafc_getheader(const unsigned char* bytes) {
    return header_valid(bytes) ? (AAFC_HEADER*)bytes : NULL;
}

EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool nm, float pitch) {
    AAFCOUTPUT output = {0,NULL};
    if (!samples || bps == 0 || sampletype == 0) {
        printf("AAFC FATAL ERROR: samples, bps or sample type not set\n");
        return output;
    }

    if (pitch == 0) pitch = 1;

    if (samplelength < 1) {
        printf("AAFC ERROR: samplelength cannot be below 1.\n");
        return output;
    }

    AAFC_HEADER* header = NULL;
    if ((header = create_header(freq, channels, samplelength, bps, sampletype)) == NULL) {
        printf("AAFC FATAL ERROR: could not allocate a new header.\n");
        return output;
    }

    float* rsptr = samples;

    if (forcemono && channels != 1)
        forceMono(rsptr, header, &channels, &samplelength);

    if ((samplerateoverride != 0 && samplerateoverride != freq) || pitch != 1)
        rsptr = resampleAudio(rsptr, header, samplerateoverride, freq, channels, &samplelength, pitch);

    if (nm) normalize(rsptr, samplelength);

    void* smpl = NULL;
    size_t audsize = 0;

    switch (sampletype) {
        case 1: {
            smpl = encode_pcm(rsptr, samplelength, &audsize, bps);
            break;
        }
        case 2: {
            if (channels > 1) rsptr = force_independent_channels(rsptr, channels, samplelength);
            smpl = encode_adpcm(rsptr, samplelength, &audsize);
            break;
        }
        case 3: {
            smpl = encode_dpcm(rsptr, samplelength, &audsize);
            break;
        }
        case 4: {
            smpl = encode_sfpcm(rsptr, samplelength, &audsize, bps);
            break;
        }
        case 5: {
            smpl = encode_ulaw(rsptr, samplelength, &audsize);
            break;
        }
        default: {
            free(header); free(rsptr);
            printf("AAFC ERROR: Invalid sample type!\n");
            return output;
        }
    }


    if (!smpl) {
        free(rsptr); free(header);
        return output;
    }

    size_t tdsize = sizeof(AAFC_HEADER) + audsize;
    unsigned char* rst = (unsigned char*)malloc(tdsize);

    memcpy(rst, header, sizeof(AAFC_HEADER));
    free(header);
    memcpy(rst + sizeof(AAFC_HEADER), smpl, audsize);
    free(smpl);
    if (rsptr != samples) free(rsptr);

    output = (AAFCOUTPUT){ tdsize, rst };
    return output;
}

EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes) {
    AAFCDECOUTPUT output = { (AAFC_HEADER){0}, NULL };
    if (!header_valid(bytes)) {
        printf("AAFC: invalid aafc data\n");
        return output;
    }

    const AAFC_HEADER* header = (const AAFC_HEADER*)bytes; // evil
    output.header = *header;

    if ((output.data = (float*)malloc(header->samplelength * sizeof(float))) == NULL) /* that's something */ {
        printf("AAFC: FAILED ALLOCATION OF DATA\n");
        output.header = (AAFC_HEADER){ 0 };
        return output;
    }
    float* rsptr = output.data;

    switch (header->sampletype) {
        case 1: {
            decode_pcm(bytes, rsptr, header->samplelength, header->bps);
            break;
        }
        case 2: {
            decode_adpcm(bytes, rsptr, header->samplelength);
            if (header->channels > 1) {
                float* itrsamples = force_interleave_channels(output.data, header->channels, header->samplelength);
                if (itrsamples) {
                    free(output.data);
                    output.data = itrsamples;
                }
            }
            break;
        }
        case 3: {
            decode_dpcm(bytes, rsptr, header->samplelength);
            break;
        }
        case 4: {
            decode_sfpcm(bytes, rsptr, header->samplelength, header->bps);
            break;
        }
        case 5: {
            decode_ulaw(bytes, rsptr, header->samplelength);
            break;
        }
        default: {
            free(output.data);
            printf("AAFC IMPORT ERROR: Invalid sample type!\n");
            output.header = (AAFC_HEADER){0};
        }
    }

    if (output.data == NULL)
        output.header = (AAFC_HEADER){0};

    return output;
}

EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end)
{
    float* samples = (float*)malloc(end * sizeof(float));
    if (legacy_header_valid(bytes)) {
        AAFC_HEADER* header = (AAFC_HEADER*)bytes;
        int sampleCount = header->samplelength;
        int bps = header->bps;
        const unsigned char* smpraw = bytes + sizeof(AAFC_HEADER);

        //TODO: support more sample types perhaps

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
            for (char* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++) {
                *sptr = (char)round(clampf(*aptr * 127.0f, -128.0f, 127.0f));
            }
            rst = csmpl;
            break;
        }
        case 16: {
            short* csmpl = (short*)malloc(size * sizeof(short));
            for (short* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++) {
                *sptr = (short)clampf(*aptr * 32767.0f, -32768.0f, 32767.0f);
            }
            rst = csmpl;
            break;
        }
        case 32: {
            int* csmpl = (int*)malloc(size * sizeof(int));
            for (int* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++) {
                *sptr = (int)clampf(*aptr * 2147483647.0f, -2147483648.0f, 2147483647.0f);
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
    float* rsptr = csmpl;

    switch (type) {
        case 8: {
            for (const char* sptr = (const char*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++) {
                *rsptr = *sptr * INT8_REC;
            }
            break;
        }
        case 16: {
            for (const short* sptr = (const short*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++) {
                *rsptr = *sptr * INT16_REC;
            }
            break;
        }
        case 32: {
            for (const int* sptr = (const int*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++) {
                *rsptr = *sptr * INT32_REC;
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

EXPORT float* aafc_resample_data(float* input, unsigned int samplerateoverride, unsigned int freq, unsigned char channels, unsigned int* samplelength, float pitch) {
    return resampleAudio(input, NULL, samplerateoverride, freq, channels, samplelength, pitch);
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
    // TODO: REWORK FILE TABLES

    AAFCOUTPUT output = { };
    return output;
}

#endif