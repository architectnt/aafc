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

#define AAFC_STRING "AAFC"
#define AAFC_SIGNATURE 0xAAFC
#define AFT_SIGNATURE 0x0ADC

// FORMATTED AS 'big SMALL TINY'
#define AAFCVERSION 214

typedef struct {
    char headr[5];
    unsigned int version;
    unsigned int freq;
    unsigned char channels;
    unsigned int samplelength;
    unsigned char bps;
    unsigned char sampletype;
} AAFC_HEADER;

typedef struct { // reserved for the next
    short signature;
    unsigned int version;
    unsigned int freq;
    unsigned char channels;
    unsigned int samplelength;
    unsigned char bps;
    unsigned char sampletype;
    unsigned int loopst;
    unsigned int loopend;
} AAFCNX_HEADER;

typedef struct {
    unsigned char* data;
    size_t size;
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
} FILETABLE;

typedef struct {
    short signature;
    unsigned int version;
    FILETABLE* filetables;
    unsigned char size;
} AAFCFILETABLE;

// Compares if the input is a valid AAFC format
bool header_valid(const unsigned char* bytes);

// Compares if the input is a valid AAFC FILE TABLE format
bool aftheader_valid(const unsigned char* bytes);

AAFC_HEADER* create_header(unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype);

bool create_aftheader(AAFCFILETABLE* t);

#endif