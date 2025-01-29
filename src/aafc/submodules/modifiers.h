/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>

void forceMono(float* input, AAFC_HEADER* h);
float* resampleAudio(float* input, AAFC_HEADER* header, unsigned long samplerateoverride, float pitch, bool nointerp);
float* forceIndependentChannels(float* input, const AAFC_HEADER* h);
float* normalize(float* input, const AAFC_HEADER* h);
float* forceInterleaveChannels(float* input, const AAFC_HEADER* h);