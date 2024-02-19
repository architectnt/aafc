#ifdef _WIN32
#define LIB_AAFC_RPATH "aafc.dll"
#elif __APPLE__
#define LIB_AAFC_RPATH "./aafc.dylib"
#else
#define LIB_AAFC_RPATH "./aafc.so"
#endif