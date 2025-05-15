/*
    Copyright (C) 2024 Architect Enterprises
    This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "helpers.h"
#include "common.h"
#include "submodules/adpcm.h"
#include "submodules/dpcm.h"
#include "submodules/pcm.h"
#include "submodules/sfpcm.h"
#include "submodules/modifiers.h"
#include "submodules/ulaw.h"

static inline void* allocSampleType(OUTPUTTYPE type, unsigned int sampleCount) {
	switch (type) {
		case OUTPUT_T_FLOAT: 
			return (float*)malloc(sampleCount * sizeof(float));
		case OUTPUT_T_UNSIGNED_BYTE:
			return (unsigned char*)malloc(sampleCount);
		case OUTPUT_T_BYTE: 
			return (signed char*)malloc(sampleCount);
		case OUTPUT_T_SHORT: 
			return (short*)malloc(sampleCount * sizeof(short));
		case OUTPUT_T_INT: 
			return (int*)malloc(sampleCount * sizeof(int));
	}
};