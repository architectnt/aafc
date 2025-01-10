/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef HELPERS_H
#define HELPERS_H 1

// Constant division (allows faster computation)

#define CLAMP(v, min, max) ((v) > (max) ? (max) : ((v) < (min) ? (min) : (v)))

//HAH
static const float INT3_REC = 1.0f / 0x03,
INT4_REC = 1.0f / 0x07,
INT7_REC = 1.0f / 0x3f,
INT8_REC = 1.0f / 0x7f,
INT10_REC = 1.0f / 0x1ff,
INT12_REC = 1.0f / 0x7ff,
INT16_REC = 1.0f / 0x7fff,
INT24_REC = 1.0f / 0x7fffff,
INT32_REC = 1.0f / (float)0x7fffffff, //fuk
SF8_REC = 1.0f / 0x0f,
PI = 3.14159274f;

#endif