/*
    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef HELPERS_H
#define HELPERS_H 1

static inline int minn(int a, int b) {
    return (a < b) ? a : b;
}

static inline int maxx(int a, int b) {
    return (a > b) ? a : b;
}

static inline int clampp(int val, int a, int b) {
    return maxx(minn(val, b), a);
}

static inline int minf(float a, float b) {
    return (a < b) ? a : b;
}

static inline int maxf(float a, float b) {
    return (a > b) ? a : b;
}

static inline int clampf(float val, float a, float b) {
    return maxf(minf(val, b), a);
}

// Constant division (allows faster computation)

//HAH
static const float INT4_REC = 1.0f / 7.0f;

static const float INT8_REC = 1.0f / 127.0f;
static const float INT10_REC = 1.0f / 511.0f;
static const float INT12_REC = 1.0f / 2047.0f;
static const float INT16_REC = 1.0f / 32767.0f;
static const float INT24_REC = 1.0f / 8388607.0f;
static const float INT32_REC = 1.0f / 2147483647.0f;
static const float SF8_REC = 1.0f / 15.0f;
static const float PI = 3.14159265359f;

#endif