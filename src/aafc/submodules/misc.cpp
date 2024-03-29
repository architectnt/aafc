/*
	Copyright (C) 2024 Architect Enterprises
	This file is a part of AAFC and is licenced under the MIT Licence.
*/

#include <iostream>

static inline void freeSamples(void* input, unsigned char bps, unsigned char sampletype) {
	if (!input) return;

	if (sampletype == 1) {
		switch (bps) {
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
	}
	else if (sampletype == 2 || sampletype == 3) {
		free((char*)input);
	}
	else if (sampletype == 4) {
		if (bps == 8) {
			free((char*)input);
		}
		else if (bps == 16) {
			free((short*)input);
		}
	}
}