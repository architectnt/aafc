/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

void forceMono(float* input, AAFC_HEADER* h);
float* resampleAudio(float* input, AAFC_HEADER* header, unsigned int samplerateoverride, float pitch, bool nointerp);
float* force_independent_channels(float* input, const AAFC_HEADER* h);
float* normalize(float* input, const AAFC_HEADER* h);
float* force_interleave_channels(float* input, const AAFC_HEADER* h);