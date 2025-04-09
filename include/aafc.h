/*
    Copyright (C) 2024-2025 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef AAFC_H
#define AAFC_H 1

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h> // shucks
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <aafc/helpers.h>
#include <aafc/common.h>


#ifdef _WIN32
    #ifdef BUILDING_SHARED_LIBRARY
        #define EXPORT inline __declspec(dllexport)
    #else
        #define EXPORT
    #endif
#else
    #if defined __GNUC__ || __clang__
        #define EXPORT __attribute__((visibility("default")))
    #else
        #define EXPORT
    #endif
#endif

#define LEGACYHEADER 0x43464141u
#define AAFC_SIGNATURE 0xAAFC // aafc fits under hexadecimal !!!
#define AFT_SIGNATURE 0x0AF7

// FORMATTED AS 'big SMALL TINY'
#define AAFCVERSION 300
#define AFTVERSION 100

typedef struct {
    unsigned short signature, version;
    unsigned long freq;
    unsigned char channels, bps, sampletype;
    unsigned long samplelength, loopst, loopend;
} AAFC_HEADER;

typedef struct { // used for older versions
    char headr[5];
    unsigned int version;
    unsigned int freq;
    unsigned char channels;
    unsigned int samplelength;
    unsigned char bps;
    unsigned char sampletype;
} AAFC_LCHEADER;

typedef struct {
    size_t size;
    unsigned char* data;
} AAFCOUTPUT;

typedef enum { // brought tragedy
    OUTPUT_T_FLOAT,
    OUTPUT_T_UNSIGNED_BYTE,
    OUTPUT_T_BYTE,
    OUTPUT_T_SHORT,
    OUTPUT_T_INT,
} OUTPUTTYPE;

typedef struct decoutput {
    AAFC_HEADER header;
    float* data;
} AAFCDECOUTPUT;

// Compares if the input is a valid format
bool legacy_header_valid(const unsigned char* bytes);
bool header_valid(const unsigned char* bytes);


// Exports
EXPORT unsigned short aafc_getversion();
EXPORT AAFC_HEADER* aafc_getheader(const unsigned char* bytes);
EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned long freq, unsigned char channels, unsigned long samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned long samplerateoverride, bool nm, float pitch, bool nointerp);
EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes);

EXPORT void* aafc_float_to_int(float* arr, long size, unsigned char type);
EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type);

EXPORT float* aafc_resample_data(float* input, unsigned long samplerateoverride, AAFC_HEADER* h, float pitch, bool nointerp);
EXPORT float* aafc_normalize(float* arr, const AAFC_HEADER* h);

// use if you're using AAFC as a shared library that isn't C/C++
EXPORT void aafc_nativefree(void* ptr);

#endif // AAFC_H