/*
    Copyright (C) 2024 Architect Enterprises
    This file is apart of AAFC and is licenced under the MIT Licence.
*/

#ifndef AAFC_H
#define AAFC_H 1

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <aafc/helpers.h>
#include <aafc/common.h>

#define HAVE_AAFC_CORE

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

#define AAFC_SIGNATURE 0xAAFC // aafc fits under hexadecimal !!!
#define AFT_SIGNATURE 0x0AF7

// FORMATTED AS 'big SMALL TINY'
#define AAFCVERSION 300
#define AFTVERSION 100

typedef struct {
    unsigned short signature;
    unsigned short version;
    unsigned int freq;
    unsigned char channels;
    unsigned char bps;
    unsigned char sampletype;
    unsigned int samplelength;
    unsigned int loopst;
    unsigned int loopend;
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

typedef struct {
    unsigned long long int len;
    unsigned char* data;
} DATATABLE;

typedef struct {
    AAFC_HEADER header;
    unsigned long long int startloc;
    char identifier[];
} AAFCTABLEDEFINITION;

typedef struct {
    unsigned short size;
    AAFCTABLEDEFINITION* table;
    DATATABLE* data;
} TABLECONTENT;

typedef struct {
    unsigned short signature;
    unsigned short version;
    unsigned char size;
    TABLECONTENT* filetables;
} AAFCFILETABLE;

// Compares if the input is a valid format
bool legacy_header_valid(const unsigned char* bytes);
bool header_valid(const unsigned char* bytes);
bool aftheader_valid(const unsigned char* bytes);

#ifdef AAFC_FUNCT
AAFC_HEADER* create_header(unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype);
bool create_aftheader(AAFCFILETABLE* t);

// Exports
EXPORT unsigned short aafc_getversion();
EXPORT AAFC_HEADER* aafc_getheader(const unsigned char* bytes);
EXPORT AAFCOUTPUT aafc_export(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool nm, float pitch);
EXPORT AAFCDECOUTPUT aafc_import(const unsigned char* bytes);

EXPORT float* aafc_chunk_read(const unsigned char* bytes, int start, int end);

EXPORT void* aafc_float_to_int(float* arr, long size, unsigned char type);
EXPORT void* aafc_int_to_float(void* arr, long size, unsigned char type);

EXPORT float* aafc_resample_data(float* input, unsigned int samplerateoverride, unsigned int freq, unsigned char channels, unsigned int* samplelength, float pitch);
EXPORT float* aafc_normalize(float* arr, int len);


//TODO: aafc content tables
#if 0
EXPORT AAFCFILETABLE aft_create(unsigned char*** data, size_t tablelength, size_t* sizes);
EXPORT AAFCOUTPUT aft_export(AAFCFILETABLE* ftable);
EXPORT AAFCFILETABLE* aft_import(unsigned char* data);
EXPORT AAFCOUTPUT aft_get_clip(AAFCFILETABLE* ftable, unsigned char group, unsigned short index);
#endif
#endif // AAFC_FUNCT

#endif // AAFC_H