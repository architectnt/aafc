#ifndef AAFC_H
#define AAFC_H 1

#ifdef _WIN32
#define EXPORT inline __declspec(dllexport)
#else
#define EXPORT
#endif

struct AAFC_HEADER {
    const char headr[5] = "AAFC";
    const int version = 2;
    int freq;
    unsigned char channels;
    int samplelength;
    unsigned char bps;
    unsigned char sampletype;
};

// Compares if the input is a valid AAFC format
bool header_valid(const unsigned char* bytes);

#endif