/*

	aud2aafc
	(dependency: libsndfile)

	Converts any format supported file into AAFC itself
	(also an excample in C/C++ how you could use AAFC)

	2024 Architect Enterprises

*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include "aafc.h"
#include <sndfile.h>
#include <string>
#include <sstream>
#include "tests.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>

// cross-platform defines
#define mkdir(dir, mode) _mkdir(dir)
#define strdup _strdup

#else
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <dirent.h>
#endif

char** list_files(const char* dir, int* len) {
	char** files = NULL;
	*len = 0;
	char abs_file_path[512];

#ifdef _WIN32
	char fullpath[512];

	snprintf(fullpath, sizeof(fullpath), "%s/*", dir);
	WIN32_FIND_DATA fdt;
	HANDLE fhndl = FindFirstFile(fullpath, &fdt);

	if (fhndl == INVALID_HANDLE_VALUE) {
		printf("cant get the first file >:((((((((((((((((: %d\n", GetLastError());
		return NULL;
	}
	do {
		if (!(fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			snprintf(abs_file_path, sizeof(abs_file_path), "%s\\%s", dir, fdt.cFileName);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(abs_file_path);
			(*len)++;
		}
	} while (FindNextFile(fhndl, &fdt) != 0);
	FindClose(fhndl);

#else
	DIR* dirc;
	struct dirent* entry;

	if (!(dirc = opendir(dir)))
		return NULL;

	while ((entry = readdir(dirc)) != NULL) {
		if (entry->d_type == DT_REG) {
			snprintf(abs_file_path, sizeof(abs_file_path), "%s/%s", dir, entry->d_name);
			files = (char**)realloc(files, (*len + 1) * sizeof(char*));
			files[*len] = strdup(abs_file_path);
			(*len)++;
		}
	}
	closedir(dirc);
#endif

	return files;
}


typedef AAFCOUTPUT (*AAFCExport)(float* samples, int freq, unsigned char channels, int samplelength, unsigned char bps, unsigned char sampletype, bool forcemono, int samplerateoverride, bool normalize, float pitch);

AAFCOUTPUT ExportAAFC(float* samples, int freq, int channels, int samplelength, unsigned char bps = 16, unsigned char sampletype = 1, bool forcemono = false, int samplerateoverride = 0, bool normalize = false, float pitch = 1) {
#ifdef _WIN32
	HMODULE aafcdll = LoadLibrary(LIB_AAFC_RPATH);
	if (aafcdll == NULL) {
		perror("Core AAFC DLL not found!");
		return {nullptr, 0};
	}

	AAFCExport aexport = (AAFCExport)GetProcAddress(aafcdll, "aafc_export");
	if (aexport == NULL) {
		perror("Could not initialize AAFC functions.");
		return { nullptr, 0 };
	}
	return aexport(samples, freq, channels, samplelength, bps, sampletype, forcemono, samplerateoverride, normalize, pitch);
#else
	void* hndl = dlopen(LIB_AAFC_RPATH, RTLD_LAZY);
	if (!hndl) {
		fprintf(stderr, "%s\n", dlerror());
		return { nullptr, 0 };
	}
	dlerror();

	AAFCExport aexport = (AAFCExport)dlsym(hndl, "aafc_export");
	char* error;
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		dlclose(hndl);
		return NULL;
	}

	return aexport(samples, freq, channels, samplelength, bps, sampletype, forcemono, samplerateoverride, normalize, pitch);
#endif
}

char* filename_without_extension(const char* path) {
	const char* base = strrchr(path, '/');
	if (!base) {
		base = strrchr(path, '\\');
	}
	if (!base) {
		base = path;
	}
	else {
		base++;
	}

	const char* ext = strrchr(base, '.');
	size_t len;
	if (!ext) {
		len = strlen(base);
	}
	else {
		len = ext - base;
	}

	char* filename = (char*)malloc(len + 1);
	if (!filename) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strncpy(filename, base, len);
	filename[len] = '\0';

	return filename;
}

char* get_dir_name(const char* path) {
	if (path == NULL || *path == '\0') {
		return NULL;
	}

	const char* last_slash = strrchr(path, '/');
	if (!last_slash) {
		last_slash = strrchr(path, '\\');
	}

	if (!last_slash) {
		return strdup(path);
	}

	if (*(last_slash + 1) == '\0') {
		while (last_slash > path) {
			last_slash--;
			if (*last_slash == '/' || *last_slash == '\\') {
				last_slash++;
				break;
			}
		}

		if (last_slash == path && (*last_slash != '/' && *last_slash != '\\')) {
			return strdup(path);
		}
	}
	else {
		last_slash++;
	}

	size_t len = strlen(last_slash);
	char* dirname = (char*)malloc(len + 1);
	if (!dirname) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strncpy(dirname, last_slash, len);
	dirname[len] = '\0';

	return dirname;
}


char* add_fext(const char* filename, const char* extension) {
	size_t filename_len = strlen(filename);
	size_t extension_len = strlen(extension);

	char* new_filename = (char*)malloc(filename_len + extension_len + 1);
	if (new_filename == NULL) {
		perror("Failed to allocate memory");
		return NULL;
	}
	strcpy(new_filename, filename);
	strcat(new_filename, extension);

	return new_filename;
}



inline static void finalize(bool usemono, size_t nitms, int channels, int samplerate, int resampleoverride, unsigned char outbps, unsigned char sampletype, AAFCOUTPUT out, FILE* ofile, float pitch) {
	// deletus everything when you dont need to calculate output externally :D
	fwrite(out.data, sizeof(unsigned char), out.size, ofile);
}

int main(int argc, char* argv[]) {
	const char* fn = "input.wav";
	unsigned char outbps = 16;
	bool usemono = false;
	bool batchconvert = false;
	bool normalize = false;
	int batchlength;
	char** batchfiles;
	char* dirnm;
	unsigned char sampletype = 1;
	int resampleoverride = 0;
	float pitch = 1;

	for (int i = 1; i < argc; i++) {
		std::string input = std::string(argv[i]);
		if (input == "-i" && i + 1 < argc) {
			fn = argv[++i];
		}
		else if (input == "--bps" && i + 1 < argc) {
			outbps = round(std::stof(argv[++i], NULL));
		}
		else if (input == "-m") {
			usemono = true;
		}
		else if (input == "--adpcm") {
			sampletype = 2;
		}
		else if (input == "--dpcm") {
			sampletype = 3;
		}
		else if (input == "-n") {
			normalize = true;
		}
		else if (input == "-p") {
			pitch = std::stof(argv[++i], NULL);
		}
		else if (input == "--sfpcm") {
			sampletype = 4;
		}
		else if (input == "--batchi") {
			batchconvert = true;
			batchfiles = list_files(argv[++i], &batchlength);
			dirnm = get_dir_name(argv[i]);
			printf("Batch converting files from %s\n", dirnm);
		}
		else if (input == "-ar" && i + 1 < argc) {
			resampleoverride = round(std::stof(argv[++i], NULL));
		}
	}
	mkdir("aafc_conversions", 0755);

	if (batchconvert) {
		std::stringstream dirp; dirp << "aafc_conversions/" << dirnm;

		mkdir(dirp.str().c_str(), 0755);

		for (int i = 0; i < batchlength; i++) {
			std::stringstream fbp; fbp << dirp.str().c_str() << "/" << filename_without_extension(batchfiles[i]) << ".aafc";

			SF_INFO info;
			memset(&info, 0, sizeof(SF_INFO));

			SNDFILE* ifl = sf_open(batchfiles[i], SFM_READ, &info);
			if (!ifl) {
				// we ignore files that arent compatible with sndfile
				continue;
			}

			size_t nitms = info.frames * info.channels;
			float* smpl = (float*)malloc(nitms * sizeof(float));
			if (smpl == NULL) {
				sf_close(ifl);
				printf("aud2aafc: failed to allocate memory D:\n");
				return -1;
			}

			sf_count_t smpsi = sf_readf_float(ifl, smpl, info.frames);
			if (smpsi != info.frames) {
				printf("aud2aafc: unexpected sample count! >:(\n");
			}

			AAFCOUTPUT out = ExportAAFC(smpl, info.samplerate, info.channels, nitms, outbps, sampletype, usemono, resampleoverride, normalize, pitch);
			if (out.data == NULL) {
				printf("aud2aafc: export failed >:(\n");
				return -2;
			}

			FILE* ofile = fopen(fbp.str().c_str(), "wb");
			if (ofile == NULL) {
				perror("aud2aafc: failed to open output file >:((((");
				return -1;
			}

			finalize(usemono, nitms, info.channels, info.samplerate, resampleoverride, outbps, sampletype, out, ofile, pitch);

			sf_close(ifl);
			free(smpl);
			free(batchfiles[i]);
		}
		free(batchfiles);
	}
	else {
		std::stringstream fp; fp << "aafc_conversions/" << filename_without_extension(fn) << ".aafc";

		SF_INFO info;
		memset(&info, 0, sizeof(SF_INFO));

		SNDFILE* ifl = sf_open(fn, SFM_READ, &info);
		if (!ifl) {
			printf("aud2aafc: failed to open file :(\n");
			return -1;
		}

		size_t nitms = info.frames * info.channels;
		float* smpl = (float*)malloc(nitms * sizeof(float));
		if (smpl == NULL) {
			sf_close(ifl);
			printf("aud2aafc: failed to allocate memory D:\n");
			return -1;
		}

		sf_count_t smpsi = sf_readf_float(ifl, smpl, info.frames);
		if (smpsi != info.frames) {
			printf("aud2aafc: unexpected sample count! >:(\n");
		}

		AAFCOUTPUT out = ExportAAFC(smpl, info.samplerate, info.channels, nitms, outbps, sampletype, usemono, resampleoverride, normalize, pitch);
		if (out.data == NULL) {
			printf("aud2aafc: export failed >:(\n");
			return -2;
		}

		FILE* ofile = fopen(fp.str().c_str(), "wb");
		if (ofile == NULL) {
			perror("aud2aafc: failed to open output file >:((((");
			return -1;
		}

		finalize(usemono, nitms, info.channels, info.samplerate, resampleoverride, outbps, sampletype, out, ofile, pitch);

		sf_close(ifl);
		free(smpl);
	}

	printf("aud2aafc: Completed conversion!\n");

	return 0;
}