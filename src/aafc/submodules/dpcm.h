#include <aafc.h>

extern inline unsigned char* encode_dpcm(float* ptr, int samplelength, size_t* audsize);
extern inline void decode_dpcm(const unsigned char* input, float* output, int sampleCount);