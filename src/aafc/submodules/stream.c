/*

    Handles clips currently being used/created


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include "stream.h"
#include <aafc.h>

AAFCSTREAM** streams;
unsigned int strc = 0; // demand? what demand? what could you do with SO MANY STREAMS LOADED AT ONCE?

AAFCSTREAM* createStream(void* data, AAFC_HEADER header, unsigned int size) {
    if (size == 0 || !data) {
        printf("invalid stream parameters\n");
    }


    AAFCSTREAM* s = (AAFCSTREAM*)malloc(sizeof(AAFCSTREAM) + size);
    if (s == NULL) {
        printf("memory alloc failure for stream\n");
        return NULL;
    }

    s->header = header;
    s->length = size;
    s->data = (void*)((char*)s + sizeof(AAFCSTREAM)); // what the hell
    s->position = 0;
    if (data && size > 0) {
        memcpy(s->data, data, size);
    }

    for (unsigned int i = 0; i < strc; ++i) {
        if (streams[i] == NULL) {
            streams[i] = s;
            return s;
        }
    }

    AAFCSTREAM** temp = (AAFCSTREAM**)realloc(streams, (strc + 1) * sizeof(AAFCSTREAM*));
    if (temp == NULL) {
        printf("memory alloc error for stream instancing\n");
        free(s);
        return NULL;
    }
    streams = temp;
    streams[strc++] = s;

    return s;
}

AAFCSTREAM* getStream(unsigned int index) {
    if (index > strc || index < 0) {
        printf("invalid stream index\n");
        return NULL;
    }

    for (unsigned i = strc; i-- > 0;) {
        if (i == index && streams[index] != NULL) {
            return streams[i];
        }
    }

    printf("stream not found\n");
    return NULL;
}

int disposeStream(AAFCSTREAM* str) {
    if (str == NULL || streams == NULL) {
        return 2;
    }

    for (unsigned i = strc; i-- > 0;) {
        if (streams[i] == str) {
            free(streams[i]);
            streams[i] = NULL;
            return 0;
        }
    }
    return 1;
}