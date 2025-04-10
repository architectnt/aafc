/*

    Main AAFC Module
    Contains everything the format has to offer


    Copyright (C) 2024-2025 Architect Enterprises
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

EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool nm, float pitch, bool nointerp) {
    AAFCOUTPUT output = {0,NULL};
    if (!samples || bps == 0 || sampletype == 0 || samplelength < 1) {
        printf("%s", "AAFC FATAL ERROR: invalid parameters\nensure samples, bps, sampletype and samplelength are set\n");
        return output;
    }
    if (pitch == 0) pitch = 1;
    AAFC_HEADER header = (AAFC_HEADER){
        AAFC_SIGNATURE, AAFCVERSION,
            freq,
            channels, bps, sampletype,
            samplelength, 0, 0
    };

    float* rsptr = samples;

    if (forcemono && channels != 1)
        forceMono(rsptr, &header);
    if ((samplerateoverride != 0 && samplerateoverride != freq) || pitch != 1)
        rsptr = resampleAudio(rsptr, &header, samplerateoverride, pitch, nointerp);
    if (nm) normalize(rsptr, &header);

    void* smpl = NULL;
    size_t audsize = 0;

    switch (sampletype) {
        case 1: smpl = encode_pcm(rsptr, &header, &audsize); break;
        case 2:
            if (header.channels > 1) rsptr = forceIndependentChannels(rsptr, &header);
            smpl = encode_adpcm(rsptr, &header, &audsize);
            break;
        case 3: smpl = encode_dpcm(rsptr, &header, &audsize); break;
        case 4: smpl = encode_sfpcm(rsptr, &header, &audsize); break;
        case 5: smpl = encode_ulaw(rsptr, &header, &audsize); break;
        default:             
            free(rsptr);
            printf("%s", "AAFC ERROR: Invalid sample type!\n");
            return output;
    }

    if (!smpl) {
        free(rsptr);
        return output;
    }

    output.size = sizeof(AAFC_HEADER) + audsize;
    output.data = (unsigned char*)malloc(output.size);

    memcpy(output.data, &header, sizeof(AAFC_HEADER));
    memcpy(output.data + sizeof(AAFC_HEADER), smpl, audsize);
    free(smpl);
    if (rsptr != samples) free(rsptr);
    rsptr = NULL;
    return output;
}

EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes) {
    AAFCDECOUTPUT output = { (AAFC_HEADER){0}, NULL };
    unsigned char offset = sizeof(AAFC_HEADER);
    if (header_valid(bytes)) output.header = *(AAFC_HEADER*)bytes; // evil
    else if (legacy_header_valid(bytes)) {
        const AAFC_LCHEADER lh = *(AAFC_LCHEADER*)bytes;
        output.header = (AAFC_HEADER){
            AAFC_SIGNATURE, (unsigned short)lh.version,
            lh.freq,
            lh.channels, lh.bps, lh.sampletype,
            lh.samplelength, 0, 0
        };
        offset = sizeof(AAFC_LCHEADER);
    } else {
        printf("%s", "AAFC: invalid aafc data\n");
        return output;
    }
    if ((output.data = (float*)malloc(output.header.samplelength * sizeof(float))) == NULL) { // that's something
        printf("%s", "AAFC: FAILED ALLOCATION OF DATA\n");
        output.header = (AAFC_HEADER){ 0 };
        return output;
    }
    float* ptr = output.data;
    const unsigned char* dt = bytes+offset;
    switch (output.header.sampletype) {
        case 1: decode_pcm(dt, ptr, &output.header); break;
        case 2:
            decode_adpcm(dt, ptr, &output.header);
            if (output.header.channels > 1) {
                float* itrsamples = forceInterleaveChannels(output.data, &output.header);
                if (itrsamples) {
                    free(output.data);
                    output.data = itrsamples;
                }
            }
            break;
        case 3: decode_dpcm(dt, ptr, &output.header); break;
        case 4: decode_sfpcm(dt, ptr, &output.header); break;
        case 5: decode_ulaw(dt, ptr, &output.header); break;
        default:
            free(output.data);
            printf("%s", "AAFC IMPORT ERROR: Invalid sample type!\n");
            output.header = (AAFC_HEADER){ 0 };
            break;
    }
    return output;
}

// compatibility layer for any subsystem that can only use integers instead of floats
EXPORT void* aafc_float_to_int(float* arr, long size, unsigned char type) {
    float* aptr = arr;
    switch (type) {
        case 8: {
            signed char* csmpl = (signed char*)malloc(size);
            for (signed char* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++)
                *sptr = (signed char)round(CLAMP(*aptr * 127.0f, -128.0f, 127.0f));
            return csmpl;
        }
        case 16: {
            short* csmpl = (short*)malloc(size * sizeof(short));
            for (short* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++)
                *sptr = (short)CLAMP(*aptr * 32767.0f, -32768.0f, 32767.0f);
            return csmpl;
        }
        case 32: {
            int* csmpl = (int*)malloc(size * sizeof(int));
            for (int* sptr = csmpl, *n = sptr + size; sptr < n; aptr++, sptr++)
                *sptr = (int)CLAMP(*aptr * 2147483647.0f, -2147483648.0f, 2147483647.0f);
            return csmpl;
        }
        default: {
            printf("unknown integer type: %d", type);
            return NULL;
        }
    }
}

// compatibility layer for systems that use integers instead (useful for exporting)
EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type) {
    float* csmpl = (float*)malloc(size * sizeof(float));
    float* rsptr = csmpl;

    switch (type) {
        case 8: {
            for (const char* sptr = (const char*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++)
                *rsptr = *sptr * INT8_REC;
            break;
        }
        case 16: {
            for (const short* sptr = (const short*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++)
                *rsptr = *sptr * INT16_REC;
            break;
        }
        case 32: {
            for (const int* sptr = (const int*)arr, *n = sptr + size; sptr < n; rsptr++, sptr++)
                *rsptr = *sptr * INT32_REC;
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

EXPORT float* aafc_resample_data(float* input, unsigned int samplerateoverride, AAFC_HEADER* h, float pitch, bool nointerp) {
    return resampleAudio(input, h, samplerateoverride, pitch, nointerp);
}

EXPORT float* aafc_normalize(float* arr, const AAFC_HEADER* h) {
    return normalize(arr, h);
}

// LKJDSKLAHNFKJLHSKAJHKJLDFHSKJAHFKLJSA MinGW LKDFHSAJLKFHKLJXHLCKJZHNJKCNHKLJNHakljdhaskh DOTNET fslkhcjkxhzknhclznckjzhnxcz
// pain and suffering
EXPORT void aafc_nativefree(void* ptr) {
    free(ptr);
}