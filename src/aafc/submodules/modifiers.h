/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

void forceMono(float* input, AAFC_HEADER* header, unsigned char* channels, unsigned int* samplelength);
float* resampleAudio(float* input, AAFC_HEADER* header, unsigned int samplerateoverride, unsigned int freq, unsigned char channels, unsigned int* samplelength, float pitch, bool nointerp);
float* force_independent_channels(float* input, const unsigned char channels, const unsigned int samplelength);
float* normalize(float* input, const unsigned int len);
float* force_interleave_channels(float* input, const unsigned char channels, const unsigned int samplelength);