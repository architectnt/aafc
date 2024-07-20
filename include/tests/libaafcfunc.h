/*

    Library import functions for tests between AAFC

*/

#include <aafc.h>
#include <string>
#include <iostream>

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

typedef AAFCDECOUTPUT (*AAFCImport)(const unsigned char*);
typedef AAFC_HEADER* (*AAFCGetHeader)(const unsigned char*);
typedef AAFCOUTPUT (*AAFCExport)(float* samples, unsigned int freq, unsigned char channels, unsigned int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, unsigned int samplerateoverride, bool normalize, float pitch);
typedef AAFCFILETABLE (*AFTCreate)(unsigned char*** data, size_t tablelength, size_t* sizes);
typedef AAFCOUTPUT (*AFTExport)(AAFCFILETABLE* ftable);
typedef AAFCFILETABLE* (*AFTImport)(unsigned char* data);


class SharedLibraryHelper{
public:
    static SharedLibraryHelper& getInstance(const std::string& dllPath) {
        static SharedLibraryHelper instance(dllPath);
        return instance;
    }

    // o        h                .                   .
    template<typename T> T getFunction(const std::string& functionName) {
        if (!handle) {
            return nullptr;
        }
#ifdef _WIN32
        T func = reinterpret_cast<T>(GetProcAddress(static_cast<HMODULE>(handle), functionName.c_str()));
        if (!func) {
            std::cerr << "function get failure: " << functionName << std::endl;
        }
#else
        T func = reinterpret_cast<T>(dlsym(handle, functionName.c_str()));
        const char* error = dlerror();
        if (error) {
            std::cerr << "function get failure: " << functionName << " - " << error << std::endl;
            func = nullptr;
        }
#endif
        return func;
    }
private:
    void* handle;

    SharedLibraryHelper(const std::string& dllPath) : handle(nullptr) {
#ifdef _WIN32
        handle = LoadLibrary(dllPath.c_str());
        if (!handle) {
            std::cerr << "couldn't load dll: " << dllPath << std::endl;
        }
#else
        handle = dlopen(dllPath.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "couldn't load .so: " << dllPath << std::endl;
        }
#endif
    }

    ~SharedLibraryHelper() {
        if (handle) {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
        }
    }

    // Delete copy constructor and assignment operator to ensure singleton property
    SharedLibraryHelper(const SharedLibraryHelper&) = delete;
    SharedLibraryHelper& operator=(const SharedLibraryHelper&) = delete;
};

AAFCDECOUTPUT LoadAAFC(const unsigned char* data) {
    SharedLibraryHelper& lib = SharedLibraryHelper::getInstance(LIB_AAFC_RPATH);
    AAFCImport aimport = lib.getFunction<AAFCImport>("aafc_import");
    if (!aimport) {
        return {};
    }
    return aimport(data);
}

AAFC_HEADER* GrabHeader(const unsigned char* data) {
    SharedLibraryHelper& lib = SharedLibraryHelper::getInstance(LIB_AAFC_RPATH);
    AAFCGetHeader aheader = lib.getFunction<AAFCGetHeader>("aafc_getheader");
    if (aheader == NULL) {
        perror("Could not initialize AAFC functions.");
        return {};
    }
    return aheader(data);
}

AAFCOUTPUT ExportAAFC(float* samples, unsigned int freq, unsigned int channels, unsigned int samplelength, unsigned char bps = 16, unsigned char sampletype = 1, bool forcemono = false, unsigned int samplerateoverride = 0, bool normalize = false, float pitch = 1) {
    SharedLibraryHelper& lib = SharedLibraryHelper::getInstance(LIB_AAFC_RPATH);
    AAFCExport aexport = lib.getFunction<AAFCExport>("aafc_export");
    if (aexport == NULL) {
        perror("Could not initialize AAFC functions.");
        return { nullptr, 0 };
    }
    return aexport(samples, freq, channels, samplelength, bps, sampletype, forcemono, samplerateoverride, normalize, pitch);
}

AAFCOUTPUT ExportAFT(AAFCFILETABLE* ftable) {
    SharedLibraryHelper& lib = SharedLibraryHelper::getInstance(LIB_AAFC_RPATH);
    AFTExport aftexp = lib.getFunction<AFTExport>("aft_export");
    if (aftexp == NULL) {
        perror("Could not initialize AAFC functions.");
        return { nullptr, 0 };
    }
    return aftexp(ftable);
}

#endif