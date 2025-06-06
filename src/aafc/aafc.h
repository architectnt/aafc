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


#ifdef _WIN32
    #ifdef BUILDING_SHARED_LIBRARY
        #define EXPORT __declspec(dllexport)
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

// FORMATTED AS 'big SMALL TINY'
#define AAFCVERSION 301
#define HEADERSIZE 21 // compact header size

typedef struct {
    unsigned short signature, version;
    unsigned int freq;
    unsigned char channels, bps, sampletype;
    unsigned int samplelength, loopst, loopend;
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


AAFC_HEADER parseHeader(const unsigned char* bytes, unsigned char* offset);

void compactHeader(unsigned char* dt, AAFC_HEADER h);


// Exports
EXPORT unsigned short aafc_getversion();
EXPORT AAFC_HEADER* aafc_getheader(const unsigned char* bytes);
EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool nm, float pitch, bool nointerp, unsigned int lstart, unsigned int lend);
EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes);

EXPORT void* aafc_float_to_int(float* arr, long size, unsigned char type);
EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type);

EXPORT float* aafc_resample_data(float* input, unsigned int samplerateoverride, AAFC_HEADER* h, float pitch, bool nointerp);
EXPORT float* aafc_normalize(float* arr, const AAFC_HEADER* h);

// use if you're using AAFC as a shared library that isn't used in a C/C++ project
EXPORT void aafc_nativefree(void* ptr);

#endif // AAFC_H