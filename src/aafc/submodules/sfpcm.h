/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

extern inline void* encode_sfpcm(float* ptr, unsigned int samplelength, size_t* audsize, unsigned char bps);
extern inline void decode_sfpcm(const unsigned char* input, float* output, unsigned int sampleCount, unsigned char bps);