/*
	Copyright (C) 2024 Architect Enterprises
	This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <aafc.h>
#include "../lib.h"

inline void* allocSampleType(OUTPUTTYPE type, int sampleCount) {
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
}

inline void freeSamples(void* input, unsigned char bps, unsigned char sampletype) {
    if (!input) return;
    switch (sampletype) {
        case 4:
        case 1:
            switch (bps) {
            case 1:
            case 4:
            case 8:
            case 10:
            case 12:
            case 24:
                free((char*)input);
                break;
            case 16:
                free((short*)input);
                break;
            case 32:
                free((float*)input);
                break;
            }
            break;
        case 2:
        case 3:
        case 5:
            free((char*)input);
            break;
    }
}