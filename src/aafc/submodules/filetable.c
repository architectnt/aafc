/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "filetable.h"

AAFCOUTPUT create_filetable_stream(AAFCFILETABLE* ftable) {
    AAFCOUTPUT output = { 0, NULL };
    if (ftable == NULL) {
        return output;
    }

    size_t tsize = sizeof(ftable->signature) + sizeof(ftable->version) + sizeof(ftable->size);
    unsigned int i, j, k;
    int64_t doffset = 0;

    for (i = 0; i < ftable->size; i++) {
        TABLECONTENT* tablecontent = ftable->filetables + i;
        tsize += sizeof(tablecontent->size);
        for (j = 0; j < tablecontent->size; j++) {
            AAFCTABLEDEFINITION* tabledef = tablecontent->table + j;
            tsize += sizeof(AAFC_HEADER) + sizeof(tabledef->startloc);
            tsize += strnlen(tabledef->identifier, 255) + 1;
        }
        for (k = 0; k < tablecontent->size; k++) {
            AAFCTABLEDEFINITION* tabledef = tablecontent->table + k;
            DATATABLE* dtb = tablecontent->data + k;
            tsize += sizeof(dtb->len) + dtb->len;
            doffset = tsize;
            tabledef->startloc = doffset;
            tsize += sizeof(dtb->len) + dtb->len;
        }
    }

    unsigned char* rst = (unsigned char*)malloc(tsize);
    unsigned char* ptr = rst;

    *(unsigned short*)ptr = ftable->signature; ptr += sizeof(ftable->signature);
    *(unsigned short*)ptr = ftable->version; ptr += sizeof(ftable->version);
    *(unsigned char*)ptr = ftable->size; ptr += sizeof(ftable->size);

    for (i = 0; i < ftable->size; i++) {
        TABLECONTENT* content = ftable->filetables + i;
        *(unsigned short*)ptr = content->size; ptr += sizeof(content->size);
        for (j = 0; j < content->size; j++) {
            AAFCTABLEDEFINITION* tabledef = content->table + j;
            memcpy(ptr, &(tabledef->header), sizeof(AAFC_HEADER)); ptr += sizeof(AAFC_HEADER);
            *(int64_t*)ptr = tabledef->startloc; ptr += sizeof(tabledef->startloc);
            size_t ilen = strnlen(tabledef->identifier, 255) + 1;
            if (ilen >= 256) { // Just..in..case....
                free(rst);
                return output;
            } 

            memcpy(ptr, tabledef->identifier, ilen); ptr += ilen;
        }
        for (k = 0; k < content->size; k++) {
            DATATABLE* dtb = content->data + k;
            *(int64_t*)ptr = dtb->len; ptr += sizeof(dtb->len);
            memcpy(ptr, dtb->data, dtb->len); ptr += dtb->len;
        }
    }

    output = (AAFCOUTPUT){tsize, rst};
    return output;
}

AAFCFILETABLE* decode_filetable_stream(unsigned char* data) {
    if (!aftheader_valid(data)) {
        printf("INVALID FILETABLE\n");
        return NULL;
    }

    AAFCFILETABLE* ftable = (AAFCFILETABLE*)malloc(sizeof(AAFCFILETABLE));
    if (!ftable) return NULL;

    unsigned char* ptr = data;

    ftable->signature = *(unsigned short*)ptr; ptr += sizeof(unsigned short);
    ftable->version = *(unsigned short*)ptr; ptr += sizeof(unsigned short);
    ftable->size = *(unsigned char*)ptr; ptr += sizeof(unsigned char);

    if ((ftable->filetables = (TABLECONTENT*)malloc(ftable->size * sizeof(TABLECONTENT))) == NULL) {
        free(ftable);
        return NULL;
    }

    unsigned int i, j, k;
    int64_t doffset = 0;

    for (i = 0; i < ftable->size; i++) {
        TABLECONTENT* content = ftable->filetables + i;
        content->size = *(unsigned short*)ptr; ptr += sizeof(unsigned short);
        if ((content->table = (AAFCTABLEDEFINITION*)malloc(content->size * sizeof(AAFCTABLEDEFINITION))) == NULL) {
            free(ftable->filetables);
            free(ftable);
            return NULL;
        }

        if ((content->data = (DATATABLE*)malloc(content->size * sizeof(DATATABLE))) == NULL) {
            free(content->table);
            free(ftable->filetables);
            free(ftable);
            return NULL;
        }

        for (j = 0; j < content->size; j++) {
            AAFCTABLEDEFINITION* tabledef = content->table + j;
            memcpy(&(tabledef->header), ptr, sizeof(AAFC_HEADER)); ptr += sizeof(AAFC_HEADER);
            tabledef->startloc = doffset;
            size_t ilen = strnlen((char*)ptr, 255) + 1;
            if (ilen >= 256) { // also check here
                free(content->data);
                free(content->table);
                free(ftable->filetables);
                free(ftable);
                return NULL;
            }

            memcpy(tabledef->identifier, ptr, ilen); ptr += ilen;
        }

        for (k = 0; k < content->size; k++) {
            DATATABLE* dtable = content->data + k;
            dtable->len = *(int64_t*)ptr; ptr += sizeof(int64_t);
            if ((dtable->data = (unsigned char*)malloc(dtable->len)) == NULL) {
                for (int l = 0; l < k; l++)
                    free(content->data[l].data);

                free(content->data);
                free(content->table);
                free(ftable->filetables);
                free(ftable);
                return NULL;
            }
            memcpy(dtable->data, ptr, dtable->len);
            ptr += dtable->len;
            doffset += dtable->len + sizeof(dtable->len);
        }
    }

    ptr = NULL;
    return ftable;
}
