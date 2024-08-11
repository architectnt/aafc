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