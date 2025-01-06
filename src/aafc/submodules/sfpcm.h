/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

void* encode_sfpcm(float* ptr, const AAFC_HEADER* h, size_t* audsize);
void decode_sfpcm(const unsigned char* input, float* output, const AAFC_HEADER* h);