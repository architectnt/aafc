#include <aafc.h>

extern inline void* encode_pcm(float* ptr, int samplelength, size_t* audsize, unsigned char bps);
extern inline void decode_pcm(const unsigned char* input, float* output, int sampleCount, unsigned char bps);