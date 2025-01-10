/*

    Handles clips currently being used/created


    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include "stream.h"
#include <aafc.h>

AAFCSTREAM* createStream(void* data, AAFC_HEADER header, unsigned int size) {
    if (size == 0 || !data) {
        printf("invalid stream parameters\n");
        return NULL;
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
    if (data && size > 0)
        memcpy(s->data, data, size);

    return s;
}

int readFromStream(AAFCSTREAM* str, void* out, unsigned int* length) {
    if (str == NULL || out == NULL || length == NULL)
        return 1;
    unsigned int remaining = str->length - str->position;
    if (remaining == 0) {
        *length = 0;
        return 2;
    }

    memcpy(out, (char*)str->data + str->position, remaining);
    str->position += remaining;
    *length = remaining;

    return 0;
}

int WriteToStream(AAFCSTREAM* str, void* input, unsigned int length, bool fixed) {
    if (str == NULL || input == NULL)
        return 1;

    if (str->position + length > str->length) {
        if(fixed) return 2;
        else {
            unsigned int newSize = str->position + length + 1024;
            AAFCSTREAM* newStr = (AAFCSTREAM*)realloc(str, sizeof(AAFCSTREAM) + newSize);
            if (newStr == NULL) {
                return 3;
            }

            str = newStr;
            str->length = newSize;
            str->data = (void*)((char*)str + sizeof(AAFCSTREAM));
        }
    }
    memcpy((char*)str->data + str->position, input, length);
    str->position += length;
    return 0;
}