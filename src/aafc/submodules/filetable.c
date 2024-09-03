/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "filetable.h"

AAFCOUTPUT create_filetable_stream(AAFCFILETABLE* ftable) {
    if (ftable == NULL) {
        AAFCOUTPUT output = { 0, NULL };
        return output;
    }

    size_t tsize = sizeof(ftable->signature) + sizeof(ftable->version) + sizeof(ftable->size);
    unsigned int i, j, k;

    for (i = 0; i < ftable->size; i++) {
        TABLECONTENT* tablecontent = ftable->filetables + i;
        tsize += sizeof(tablecontent->size);
        for (j = 0; j < tablecontent->size; j++) {
            AAFCTABLEDEFINITION* tabledef = tablecontent->table + j;
            tsize += sizeof(AAFC_HEADER) + sizeof(tabledef->startloc);
            tsize += strnlen(tabledef->identifier, 255) + 1;
        }
        for (k = 0; k < tablecontent->size; k++) {
            DATATABLE* dtb = tablecontent->data + k;
            tsize += sizeof(dtb->len) + dtb->len;
        }
    }

    unsigned char* rst = (unsigned char*)malloc(tsize);
    unsigned char* ptr = rst;

    *(unsigned short*)ptr = ftable->signature; ptr += sizeof(ftable->signature);
    *(unsigned short*)ptr = ftable->version; ptr += sizeof(ftable->version);
    *(unsigned char*)ptr = ftable->size; ptr += sizeof(ftable->size);

    for (i = 0; i < ftable->size; i++) {
        TABLECONTENT* tablecontent = ftable->filetables + i;
        *(unsigned short*)ptr = tablecontent->size; ptr += sizeof(tablecontent->size);
        for (j = 0; j < tablecontent->size; j++) {
            AAFCTABLEDEFINITION* tabledef = tablecontent->table + j;
            memcpy(ptr, &(tabledef->header), sizeof(AAFC_HEADER)); ptr += sizeof(AAFC_HEADER);
            *(int64_t*)ptr = tabledef->startloc; ptr += sizeof(tabledef->startloc);
            size_t ilen = strnlen(tabledef->identifier, 255) + 1;
            memcpy(ptr, tabledef->identifier, ilen); ptr += ilen;
        }
        for (k = 0; k < tablecontent->size; k++) {
            DATATABLE* dtb = tablecontent->data + k;
            *(int64_t*)ptr = dtb->len; ptr += sizeof(dtb->len);
            memcpy(ptr, dtb->data, dtb->len); ptr += dtb->len;
        }
    }

    AAFCOUTPUT output = {tsize, rst};
    return output;
}

AAFCFILETABLE* decode_filetable_stream(unsigned char* data) {
    if (aftheader_valid(data)) {
        AAFCFILETABLE* ftable = (AAFCFILETABLE*)malloc(sizeof(AAFCFILETABLE));

        // TODO: REWORK FILE TABLES

        return ftable;
    }
    else {
        printf("INVALID FILETABLE");
        return NULL;
    }
}
