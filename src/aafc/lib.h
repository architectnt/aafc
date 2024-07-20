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

extern inline void freeSamples(void* input, unsigned char bps, unsigned char sampletype);
extern inline void* allocSampleType(OUTPUTTYPE type, int sampleCount);