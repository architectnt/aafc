#include <aafc.h>
/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

AAFCSTREAM* initializeStream(AAFC_HEADER header, unsigned long size);
int readFromStream(AAFCSTREAM* str, void* out, unsigned long* length);
int WriteToStream(AAFCSTREAM* str, void* input, unsigned long length, bool fixed);