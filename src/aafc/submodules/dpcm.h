/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include "../aafc.h"

unsigned char* encode_dpcm(float* ptr, AAFC_HEADER* h, size_t* audsize);
void decode_dpcm(const unsigned char* input, float* output, const AAFC_HEADER* h);