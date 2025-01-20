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
        printf("AAFC FATAL ERROR: invalid parameters\nensure samples, bps, sampletype and samplelength are set\n");
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
            if (channels > 1) rsptr = forceIndependentChannels(rsptr, &header);
            smpl = encode_adpcm(rsptr, &header, &audsize);
            break;
        case 3: smpl = encode_dpcm(rsptr, &header, &audsize); break;
        case 4: smpl = encode_sfpcm(rsptr, &header, &audsize); break;
        case 5: smpl = encode_ulaw(rsptr, &header, &audsize); break;
        default:             
            free(rsptr);
            printf("AAFC ERROR: Invalid sample type!\n");
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
        printf("AAFC: invalid aafc data\n");
        return output;
    }
    if ((output.data = (float*)malloc(output.header.samplelength * sizeof(float))) == NULL) { // that's something
        printf("AAFC: FAILED ALLOCATION OF DATA\n");
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
            printf("AAFC IMPORT ERROR: Invalid sample type!\n");
            output.header = (AAFC_HEADER){ 0 };
            break;
    }
    return output;
}

// TODO: REPLACE
EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end) {
    printf("deprecated function: awaiting AAFC3 STREAMS\n");
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

EXPORT float* aafc_resample_data(float* input, unsigned int samplerateoverride, AAFC_HEADER* h, float pitch, bool nointerp) {
    return resampleAudio(input, h, samplerateoverride, pitch, nointerp);
}

EXPORT float* aafc_normalize(float* arr, const AAFC_HEADER* h) {
    return normalize(arr, h);
}

EXPORT AAFCOUTPUT aft_export(AAFCFILETABLE* ftable) {
    return create_filetable_stream(ftable);
}

EXPORT AAFCFILETABLE* aft_import(unsigned char* data) {
    return decode_filetable_stream(data);
}

EXPORT AAFCOUTPUT aft_get_clip_from_index(AAFCFILETABLE* ftable, unsigned char group, unsigned short index) {
    AAFCOUTPUT output = { 0, NULL };
    if (!ftable || group >= ftable->size) {
        printf("invalid group\n");
        return output;
    }

    TABLECONTENT* content = ftable->filetables + group;
    if (index >= content->size) {
        printf("invalid table\n");
        return output;
    }

    AAFCTABLEDEFINITION* def = content->table + index;
    DATATABLE* dtb = content->data + index;

    size_t csize = sizeof(AAFC_HEADER) + dtb->len;
    unsigned char* rst = (unsigned char*)malloc(csize);
    if (!rst) {
        printf("INTERNAL ALLOCATION ERROR\n");
        return output;
    }

    memcpy(rst, &(def->header), sizeof(AAFC_HEADER));
    memcpy(rst + sizeof(AAFC_HEADER), dtb->data, dtb->len);

    output = (AAFCOUTPUT){ csize, rst };
    return output;
}

EXPORT AAFCOUTPUT aft_get_clip_from_name(AAFCFILETABLE* ftable, unsigned char group, const char* identifier) {
    AAFCOUTPUT output = { 0, NULL };
    if (!ftable || !identifier || group >= ftable->size) {
        printf("invalid inputs\n");
        return output;
    }

    TABLECONTENT* content = ftable->filetables + group;
    for (unsigned short i = 0; i < content->size; i++) {
        AAFCTABLEDEFINITION* def = content->table + i;
        if (strncmp(def->identifier, identifier, 255) == 0) {
            DATATABLE* dtb = content->data + i;

            size_t csize = sizeof(AAFC_HEADER) + dtb->len;
            unsigned char* rst = (unsigned char*)malloc(csize);
            if (!rst) {
                printf("INTERNAL ALLOCATION ERROR\n");
                return output;
            }

            memcpy(rst, &(def->header), sizeof(AAFC_HEADER));
            memcpy(rst + sizeof(AAFC_HEADER), dtb->data, dtb->len);

            output = (AAFCOUTPUT){ csize, rst };
            return output;
        }
    }

    printf("identifier invalid\n");
    return output;
}

EXPORT AAFCFILETABLE aft_create(AFTINPUT data[], unsigned char grouplength) {
    AAFCFILETABLE ftable = {};
    if (!data || grouplength == 0) {
        printf("Invalid input\n");
        return ftable;
    }

    ftable.signature = AFT_SIGNATURE;
    ftable.version = AFTVERSION;
    ftable.size = grouplength;

    if ((ftable.filetables = (TABLECONTENT*)malloc(grouplength * sizeof(TABLECONTENT))) == NULL) {
        printf("Memory allocation failed\n");
        return ftable;
    }

    unsigned char i;
    unsigned short j;
    int64_t doffset = 0;

    for (i = 0; i < grouplength; i++) {
        AFTINPUT* input = &data[i];
        TABLECONTENT* tablecontent = &ftable.filetables[i];
        tablecontent->size = input->len;

        if ((tablecontent->table = (AAFCTABLEDEFINITION*)malloc(input->len * sizeof(AAFCTABLEDEFINITION))) == NULL
            || (tablecontent->data = (DATATABLE*)malloc(input->len * sizeof(DATATABLE))) == NULL) {
            free(ftable.filetables);
            return ftable;
        }

        for (j = 0; j < input->len; j++) {
            AFTSUBINPUT* subinput = &input->table[j];
            AAFCTABLEDEFINITION* tabledef = &tablecontent->table[j];
            DATATABLE* dtb = &tablecontent->data[j];
            memcpy(&tabledef->header, subinput->data, sizeof(AAFC_HEADER));
            tabledef->startloc = doffset;
            dtb->len = subinput->len - sizeof(AAFC_HEADER);
            if ((dtb->data = (unsigned char*)malloc(dtb->len)) == NULL) {
                free(tablecontent->table);
                free(tablecontent->data);
                free(ftable.filetables);
                return ftable;
            }
            memcpy(dtb->data, subinput->data + sizeof(AAFC_HEADER), dtb->len);
            size_t ilen = strnlen(subinput->identifier, 255);
            if (ilen >= 255) {
                free(tablecontent->table);
                free(dtb->data);
                free(tablecontent->data);
                free(ftable.filetables);
                return ftable;
            }
            memcpy(tabledef->identifier, subinput->identifier, ilen);
            tabledef->identifier[ilen] = '\0';
            doffset += dtb->len + sizeof(dtb->len);
        }
    }

    return ftable;
}