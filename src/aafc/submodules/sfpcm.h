/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

extern inline void* encode_sfpcm(float* ptr, int samplelength, size_t* audsize, unsigned char bps);
extern inline void decode_sfpcm(const unsigned char* input, float* output, int sampleCount, unsigned char bps);