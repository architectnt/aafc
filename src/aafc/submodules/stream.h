#include <aafc.h>
/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

AAFCSTREAM* createStream(void* data, AAFC_HEADER header, unsigned int size);
int readFromStream(AAFCSTREAM* str, void* out, unsigned int* length);
int WriteToStream(AAFCSTREAM* str, void* input, unsigned int length, bool fixed);