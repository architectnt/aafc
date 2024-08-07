/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#define HAVE_AAFC

#include <aafc.h>
#include "submodules/adpcm.h"
#include "submodules/dpcm.h"
#include "submodules/pcm.h"
#include "submodules/sfpcm.h"
#include "submodules/modifiers.h"
#include "submodules/filetable.h"
#include "submodules/ulaw.h"

// Exports

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

static inline void* allocSampleType(OUTPUTTYPE type, unsigned int sampleCount) 
{
	switch (type) {
	case OUTPUT_T_FLOAT:
		return malloc(sampleCount * sizeof(float));
		break;
	case OUTPUT_T_UNSIGNED_BYTE:
		return malloc(sampleCount);
		break;
	case OUTPUT_T_BYTE:
		return malloc(sampleCount);
		break;
	case OUTPUT_T_SHORT:
		return malloc(sampleCount * sizeof(short));
		break;
	case OUTPUT_T_INT:
		return malloc(sampleCount * sizeof(int));
		break;
	}
};