/*

    Library import functions for tests between AAFC

*/

#include <aafc.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef _WIN32
#define LIB_AAFC_RPATH "aafc.dll"
#elif __APPLE__
#define LIB_AAFC_RPATH "./libaafc.dylib"
#else
#define LIB_AAFC_RPATH "./libaafc.so"
#endif

#ifndef LIBAAFCFUNC_H
#define LIBAAFCFUNC_H 1

void* afh;

typedef AAFCDECOUTPUT (*AAFCImport)(const unsigned char*);
typedef AAFC_HEADER* (*AAFCGetHeader)(const unsigned char*);
typedef AAFCOUTPUT (*AAFCExport)(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool normalize, float pitch, bool nointerp);

static void UnloadAAFC() {
    if (afh) {
#ifdef _WIN32
        FreeLibrary((HMODULE)afh);
#else
        dlclose(afh);
#endif
        afh = NULL;
    }
}

static int LoadAAFCLib() {
#ifdef _WIN32
    afh = LoadLibraryA(LIB_AAFC_RPATH);
#else
    afh = dlopen(LIB_AAFC_RPATH, RTLD_LAZY);
#endif
    if (afh) {
        atexit(UnloadAAFC);
        return 1;
    }
    else {
        fprintf(stderr, "%s", "failed to load aafc.\n");
        return 0;
    }
}

void* LoadAAFCFunc(void* handle, const char* n) {
    if (!afh) {
        fprintf(stderr, "%s", "AAFC is NOT loaded! Please call LoadAAFCLib() before you call LoadAAFCFunc().\n");
        return NULL;
    }
#ifdef _WIN32
    FARPROC func = GetProcAddress((HMODULE)afh, n);
    if (!func) {
        fprintf(stderr, "AAFC func get error: %s\n", n);
        return NULL;
    }
    return (void*)func;
#else
    dlerror(); // Clear previous errors
    void* func = dlsym(afh, n);
    const char* error = dlerror();
    if (error) {
        fprintf(stderr, "AAFC func get error: %s\n(%s)\n", n, error);
        return NULL;
    }
    return func;
#endif
}

AAFCDECOUTPUT LoadAAFC(const unsigned char* data) {
    AAFCImport aimport = (AAFCImport)LoadAAFCFunc(afh, "aafc_import");
    if (!aimport) {
        perror("Could not initialize AAFC functions.");
        return (AAFCDECOUTPUT) { (AAFC_HEADER) { 0 }, NULL };
    }
    return aimport(data);
}

AAFC_HEADER* GrabHeader(const unsigned char* data) {
    AAFCGetHeader aheader = (AAFCGetHeader)LoadAAFCFunc(afh, "aafc_getheader");
    if (aheader == NULL) {
        perror("Could not initialize AAFC functions.");
        return NULL;
    }
    return aheader(data);
}

AAFCOUTPUT ExportAAFC(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool normalize, float pitch, bool nointerp) {
    AAFCExport aexport = (AAFCExport)LoadAAFCFunc(afh, "aafc_export");
    if (aexport == NULL) {
        perror("Could not initialize AAFC functions.");
        return (AAFCOUTPUT){0, NULL};
    }
    return aexport(samples, freq, channels, samplelength, bps, sampletype, forcemono, samplerateoverride, normalize, pitch, nointerp);
}

#endif