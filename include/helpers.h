/*
    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef HELPERS_H
#define HELPERS_H 1

#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(value, lower, upper) (Max(Min(value, upper), lower))

// Constant division (allows faster computation)

//HAH
constexpr float INT4_REC = 1.0f / 7.0f;

constexpr float INT8_REC = 1.0f / 127.0f;
constexpr float INT10_REC = 1.0f / 511.0f;
constexpr float INT12_REC = 1.0f / 2047.0f;
constexpr float INT16_REC = 1.0f / 32767.0f;
constexpr float INT24_REC = 1.0f / 8388607.0f;
constexpr float SF8_REC = 1.0f / 15.0f;
constexpr float PI = 3.14159265359f;

#endif