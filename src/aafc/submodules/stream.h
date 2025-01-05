#include <aafc.h>
/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

AAFCSTREAM* createStream(void* data, AAFC_HEADER header, unsigned int size);
int disposeStream(AAFCSTREAM* str);