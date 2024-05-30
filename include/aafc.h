/*
    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef AAFC_H
#define AAFC_H 1

#include <cstring>
#include <cmath>

#ifdef _WIN32
#define EXPORT inline __declspec(dllexport)
#else
#define EXPORT
#endif

#define AAFC_STRING "AAFC"

// FORMATTED AS 'big SMALL TINY'
#define AAFCVERSION 21

struct AAFC_HEADER {
    char headr[5];
    int version;
    int freq;
    unsigned char channels;
    int samplelength;
    unsigned char bps;
    unsigned char sampletype;
};

struct AAFCOUTPUT
{
    unsigned char* data;
    size_t size;
};

// Compares if the input is a valid AAFC format
bool header_valid(const unsigned char* bytes);

bool create_header(AAFC_HEADER* h, int freq, unsigned char channels, int samplelength, unsigned char bps, unsigned char sampletype);

#endif