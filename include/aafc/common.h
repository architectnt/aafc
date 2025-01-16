/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef COMMON_H
#define COMMON_H 1

float smoothInterpolate(float y0, float y1, float y2, float y3, double t);
unsigned char minifloat(float val);
unsigned short halfpercision(float val);
float dminif(unsigned char val);
float dhalf(unsigned short val);

#endif