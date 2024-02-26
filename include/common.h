/*
    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef COMMON_H
#define COMMON_H 1

float interpolate(float x0, float x1, float alpha);

float cubic_interpolate(float y0, float y1, float y2, float y3, double mu);

// Convert to a minifloat implementation (also what the actual HARGH is going on)
unsigned char minifloat(float val);

// Convert to a 16-bit float implementation (WHY IS THERE NO STANDARD)
unsigned short halfpercision(float val);

// this.......
float dminif(unsigned char val);

// is getting out of hand
float dhalf(unsigned short val);

float sinc(float x);

float sinc_interpolate(float* samples, int sampleCount, double t, int windowSize);

#endif