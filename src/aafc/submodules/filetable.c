/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "filetable.h"

inline AAFCOUTPUT create_filetable_stream(AAFCFILETABLE* ftable) {
    // TODO: REWORK FILE TABLES

    AAFCOUTPUT output = { };
    return output;
}

inline AAFCFILETABLE* decode_filetable_stream(unsigned char* data) {
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
