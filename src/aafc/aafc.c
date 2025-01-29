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

EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned long freq, unsigned char channels, unsigned long samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned long samplerateoverride, bool nm, float pitch, bool nointerp) {
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

// TODO: REPLACE
EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end) {
    printf("%s", "deprecated function: awaiting AAFC3 STREAMS\n");
    return NULL;
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

EXPORT float* aafc_resample_data(float* input, unsigned long samplerateoverride, AAFC_HEADER* h, float pitch, bool nointerp) {
    return resampleAudio(input, h, samplerateoverride, pitch, nointerp);
}

EXPORT float* aafc_normalize(float* arr, const AAFC_HEADER* h) {
    return normalize(arr, h);
}

EXPORT AAFCOUTPUT aft_export(AAFCTABLE* ftable) {
    return serializeTableContent(ftable);
}

EXPORT AAFCTABLE* aft_import(unsigned char* data) {
    return deserializeTableContent(data);
}

EXPORT AAFCOUTPUT aft_get_clip_from_index(AAFCTABLE* ftable, unsigned char group, unsigned short index) {
    AAFCOUTPUT output = { 0, NULL };
    if (!ftable || group >= ftable->groupsize) {
        printf("%s", "invalid group\n");
        return output;
    }

    TableAttribute* content = ftable->attributes + group;
    if (index >= content->tablesize) {
        printf("%s", "invalid table\n");
        return output;
    }

    TableDef* def = content->table + index;

    size_t csize = sizeof(AAFC_HEADER) + def->size;
    unsigned char* rst = (unsigned char*)malloc(csize);
    if (!rst) {
        printf("%s", "INTERNAL ALLOCATION ERROR\n");
        return output;
    }

    memcpy(rst, &(def->header), sizeof(AAFC_HEADER));
    memcpy(rst + sizeof(AAFC_HEADER), ftable->data + def->loc, def->size);

    output = (AAFCOUTPUT){ csize, rst };
    return output;
}

EXPORT AAFCOUTPUT aft_get_clip_from_name(AAFCTABLE* ftable, unsigned char group, const char* identifier) {
    AAFCOUTPUT output = { 0, NULL };
    if (!ftable || !identifier || group >= ftable->groupsize) {
        printf("%s", "invalid inputs\n");
        return output;
    }

    TableAttribute* content = ftable->attributes + group;
    for (unsigned short i = 0; i < content->tablesize; i++) {
        TableDef* def = content->table + i;
        if (strncmp(def->identifier, identifier, 255) == 0) {
            TableDef* def = content->table + i;

            size_t csize = sizeof(AAFC_HEADER) + def->size;
            unsigned char* rst = (unsigned char*)malloc(csize);
            if (!rst) {
                printf("%s", "INTERNAL ALLOCATION ERROR\n");
                return output;
            }

            memcpy(rst, &def->header, sizeof(AAFC_HEADER));
            memcpy(rst + sizeof(AAFC_HEADER), ftable->data + def->loc, def->size);

            output = (AAFCOUTPUT){ csize, rst };
            return output;
        }
    }

    printf("%s", "identifier invalid\n");
    return output;
}

EXPORT AAFCTABLE aft_create(AFTInput data[], unsigned char grouplength) {
    AAFCTABLE ftable = {};
    if (!data || grouplength == 0) {
        printf("%s", "Invalid input\n");
        return ftable;
    }

    ftable.signature = AFT_SIGNATURE;
    ftable.version = AFTVERSION;
    ftable.groupsize = grouplength;

    ftable.attributes = (TableAttribute*)calloc(grouplength, sizeof(TableAttribute));
    if (!ftable.attributes) return ftable;
    memset(ftable.attributes, 0, grouplength * sizeof(TableAttribute));

    uint64_t dsize = 0;
    for (unsigned char i = 0; i < grouplength; i++) {
        if (data->len > 0) {
            ftable.attributes[i].table = (TableDef*)calloc(data->len, sizeof(TableDef));
            if (!data->len) {
                printf("%s", "could not allocate attributes\n");
                free(ftable.attributes);
                return (AAFCTABLE) { 0 };
            }
        }
        else {
            printf("%s", "invalid length of tables\n");
            free(ftable.attributes);
            return (AAFCTABLE) { 0 };
        }

        uint64_t offset = 0;
        for (unsigned short j = 0; j < data->len; j++) {
            if (!header_valid(data->table[j].data)) {
                printf("%s", "one of entries has invalid AAFC data, aborting.\n");
                free(ftable.attributes);
                return (AAFCTABLE) { 0 };
            }

            memcpy(
                &ftable.attributes[i].table[j].header, 
                (AAFC_HEADER*)data->table[j].data, sizeof(AAFC_HEADER)
            );
            memcpy(
                &ftable.attributes[i].table[j].identifier,
                (AAFC_HEADER*)data->table[j].identifier, 256
            );
            ftable.attributes[i].table[j].loc = offset;
            long ln = data->table[j].len - sizeof(AAFC_HEADER); // signed to check invalid data again
            if (ln <= 0) {
                printf("%s", "WHAT\nlength of data became negative\n");
                free(ftable.attributes);
                return (AAFCTABLE) { 0 };
            }

            ftable.attributes[i].table[j].size = ln;
            offset += ln;
            dsize += ln;
        }
    }

    ftable.data = (unsigned char*)malloc(dsize);
    unsigned char* ptr = ftable.data;

    if (!ftable.data) {
        free(ftable.attributes);
        return ftable;
    }
    for (unsigned char i = 0; i < grouplength; i++) {
        for (unsigned short j = 0; j < data->len; j++) {
            long ln = data->table[j].len - sizeof(AAFC_HEADER);
            memcpy(ftable.data, data->table[j].data + sizeof(AAFC_HEADER), ln);
            ptr += ln;
        }
    }

    return ftable;
}