/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "filetable.h"

AAFCOUTPUT serializeTableContent(AAFCTABLE* ftable) {
    AAFCOUTPUT output = { 0, NULL };
    if (ftable == NULL) {
        return output;
    }

    uint64_t len = 0;
    unsigned short tlen = 0;
    unsigned long i, j;
    for (i = 0; i < ftable->groupsize; i++) {
        for (j = 0; j < ftable->attributes[i].tablesize; j++) {
            len += ftable->attributes[i].table[j].size;
        }
        tlen += ftable->attributes[i].tablesize;
    }

    uint64_t tsize = 6;
    for (unsigned long i = 0; i < ftable->groupsize; i++) {
        tsize += 64 + 2;
        tsize += (sizeof(AAFC_HEADER) + 8 + 4 + 256) * ftable->attributes[i].tablesize;
    }
    tsize += len; // Data section
    unsigned char* rst = (unsigned char*)malloc(tsize);
    memset(rst, 0, tsize); // zero out ERYryTHIG

    unsigned char* ptr = rst;

    *(unsigned short*)ptr = ftable->signature; ptr += 2;
    *(unsigned short*)ptr = ftable->version; ptr += 2;
    *ptr++ = ftable->groupsize;
    *ptr++ = ftable->compressiontype;

    uint64_t offset = 0;
    for (i = 0; i < ftable->groupsize; i++) {
        unsigned short cln = strnlen(ftable->attributes[i].identifier, 63) + 1;
        memcpy(ptr, ftable->attributes[i].identifier, cln); ptr += 64;
        *(unsigned short*)ptr = ftable->attributes[i].tablesize; ptr += 2;
        for (j = 0; j < ftable->attributes[i].tablesize; j++) {
            *(AAFC_HEADER*)ptr = ftable->attributes[i].table[j].header; ptr += sizeof(AAFC_HEADER);
            *(uint64_t*)ptr = offset; ptr += 8;
            *(unsigned long*)ptr = ftable->attributes[i].table[j].size; ptr += 4;
            unsigned short cln = strnlen(ftable->attributes[i].table[j].identifier, 255) + 1;
            memcpy(ptr, ftable->attributes[i].table[j].identifier, cln); ptr += 256;

            offset += ftable->attributes[i].table[j].size;
        }
    }

    memcpy(ptr, ftable->data, len);

    output = (AAFCOUTPUT){tsize, rst};
    return output;
}

AAFCTABLE* deserializeTableContent(unsigned char* data) {
    if (!aftheader_valid(data)) {
        printf("INVALID FILETABLE\n");
        return NULL;
    }

    AAFCTABLE* ftable = (AAFCTABLE*)malloc(sizeof(AAFCTABLE));
    if (!ftable) return NULL;

    unsigned char* ptr = data;
    unsigned char err = 0;

    ftable->signature = *(unsigned short*)ptr; ptr += 2;
    ftable->version = *(unsigned short*)ptr; ptr += 2;
    ftable->groupsize = *(unsigned char*)ptr; ptr++;
    ftable->compressiontype = *(unsigned char*)ptr; ptr++;
    unsigned long i, j;

    ftable->attributes = (TableAttribute*)calloc(ftable->groupsize, sizeof(TableAttribute));
    if (!ftable->attributes) err = 1;

    if(ftable->attributes) 
        memset(ftable->attributes, 0, ftable->groupsize * sizeof(TableAttribute));

    for (i = 0; i < ftable->groupsize && !err; i++) {
        if (ftable->attributes[i].tablesize > 0) {
            ftable->attributes[i].table = (TableDef*)calloc(
                ftable->attributes[i].tablesize, sizeof(TableDef));
            if (!ftable->attributes[i].table) {
                err = 1;
                break;
            }
        }
        else {
            err = 1;
            break;
        }

        strncpy(ftable->attributes[i].identifier, (char*)ptr, 63);
        ftable->attributes[i].identifier[63] = '\0';
        ptr += 64;

        ftable->attributes[i].tablesize = *(unsigned short*)ptr; ptr += 2;

        for (j = 0; j < ftable->attributes->tablesize && !err; j++) {
            memcpy(&ftable->attributes[i].table[j].header, ptr, sizeof(AAFC_HEADER));
            ptr += sizeof(AAFC_HEADER);

            ftable->attributes[i].table[j].loc = *(uint64_t*)ptr; ptr += 8;
            ftable->attributes[i].table[j].size = *(unsigned long*)ptr; ptr += 4;

            strncpy(ftable->attributes[i].table[j].identifier, (char*)ptr, 255);
            ftable->attributes[i].table[j].identifier[255] = '\0';
            ptr += 256;
        }
    }

    if (!err) {
        uint64_t data_size = (ptr - data) - (
            6 +
            (ftable->groupsize * 2) +
            (ftable->groupsize * ftable->attributes[0].tablesize * (sizeof(AAFC_HEADER) + 12 + 255))
            );

        ftable->data = (unsigned char*)malloc(data_size);
        if (!ftable->data) err = 1;
        else memcpy(ftable->data, ptr, data_size);
    }

    if (err) {
        if (ftable->attributes) {
            for (unsigned i = 0; i < ftable->groupsize; i++) {
                free(ftable->attributes[i].table);
            }
            free(ftable->attributes);
        }
        free(ftable);
        return NULL;
    }

    return ftable;
}
