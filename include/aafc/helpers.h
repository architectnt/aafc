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

static inline float minf(float a, float b) {
    return (a < b) ? a : b;
}

static inline float maxf(float a, float b) {
    return (a > b) ? a : b;
}

static inline float clampf(float val, float a, float b) {
    return maxf(minf(val, b), a);
}

// Constant division (allows faster computation)

//HAH
static const float INT4_REC = 1.0f / 0x07;

static const float INT7_REC = 1.0f / 0x3f;
static const float INT8_REC = 1.0f / 0x7f;
static const float INT10_REC = 1.0f / 0x1ff;
static const float INT12_REC = 1.0f / 0x7ff;
static const float INT16_REC = 1.0f / 0x7fff;
static const float INT24_REC = 1.0f / 0x7fffff;
static const float INT32_REC = 1.0f / (float)0x7fffffff; //fuk
static const float SF8_REC = 1.0f / 0x0f;
static const float PI = 3.14159274f;

#endif