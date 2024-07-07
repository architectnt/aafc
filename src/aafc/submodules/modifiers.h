#include <aafc.h>

extern inline void forceMono(float* input, AAFC_HEADER* header, unsigned char* channels, int* samplelength);
extern inline float* resampleAudio(float* input, AAFC_HEADER* header, int samplerateoverride, int freq, unsigned char channels, int* samplelength, float pitch);
extern inline float* force_independent_channels(float* input, unsigned char channels, int samplelength);
extern inline float* normalize(float* input, int len);
extern inline float* force_interleave_channels(float* input, unsigned char channels, int samplelength);