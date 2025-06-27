/*

	aud2aafc
	(dependency: libsndfile)

	Converts any format supported file into AAFC itself
	(also an example in C/C++ how you could use AAFC for exporting)

	2024-2024 Architect Enterprises

*/

#include <stdlib.h>
#include <stdio.h>
#include <sndfile.h>
#include "../../aafc/aafc.h"
#include "../fileutils.h"
#include "../libaafcfunc.h"

typedef struct {
	int statuscode;
	char* message;
} ConversionResult;

ConversionResult convertmedia(const char* fn, const char* outpath, bool usemono, bool normalize, unsigned char bps, unsigned char sampletype, unsigned long spoverride, float pitch, bool nointerp) {
	ConversionResult s = { 0, NULL};

	SF_INFO info;
	memset(&info, 0, sizeof(SF_INFO));

	SNDFILE* ifl = sf_open(fn, SFM_READ, &info);
	if (!ifl) {
		s.message = (char*)"failed to open file :("; // char cast whhh
		s.statuscode = -1;
		return s;
	}

	size_t nitms = info.frames * info.channels;
	float* smpl = (float*)malloc(nitms * sizeof(float));
	if (smpl == NULL) {
		sf_close(ifl);
		s.message = (char*)"failed to allocate memory D:";
		s.statuscode = -2;
		return s;
	}

	if (sf_readf_float(ifl, smpl, info.frames) != info.frames) {
		s.message = (char*)"result has an unmatched sample count";
		s.statuscode = 1;
	}

	AAFCOUTPUT out = ExportAAFC(smpl, info.samplerate, info.channels, nitms, bps, sampletype, usemono, spoverride, normalize, pitch, nointerp, 0, 0);
	if (out.data == NULL) {
		sf_close(ifl);
		free(smpl);
		s.message = (char*)"EXPORT FAILED >:(";
		s.statuscode = -3;
		return s;
	}

	FILE* ofile = fopen(outpath, "wb");
	if (ofile == NULL) {
		sf_close(ifl);
		free(smpl);
		s.message = (char*)"could not open file >:(((((((((((";
		s.statuscode = -4;
		return s;
	}

	fwrite(out.data, sizeof(unsigned char), out.size, ofile);

	sf_close(ifl);
	free(smpl);

	fclose(ofile);
	free(out.data);
	return s;
}

int main(int argc, char* argv[]) {
	const char* fn = "input.wav";
	unsigned char outbps = 16;
	bool usemono = false, nointerp = false, normalize = false;
	unsigned int batchlength = 0;
	char** batchfiles = NULL;
	const char* dirnm;
	unsigned char sampletype = 1;
	unsigned int resampleoverride = 0;
	float pitch = 1;
	const char* outpath = "aafc_conversions";
	char* ofn = NULL;

	for (unsigned int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
			fn = argv[++i];
		}
		if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			outpath = argv[++i];
		}
		if (strcmp(argv[i], "-fn") == 0 && i + 1 < argc) {
			ofn = argv[++i];
		}
		else if (strcmp(argv[i], "--bps") == 0 && i + 1 < argc) {
			outbps = (unsigned char)atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-m") == 0) {
			usemono = true;
		}
		else if (strcmp(argv[i], "-nointerp") == 0) {
			nointerp = true;
		}
		else if (strcmp(argv[i], "--adpcm") == 0) {
			sampletype = 2;
		}
		else if (strcmp(argv[i], "--dpcm") == 0) {
			sampletype = 3;
		}
		else if (strcmp(argv[i], "-n") == 0) {
			normalize = true;
		}
		else if (strcmp(argv[i], "-p") == 0) {
			pitch = (float)atof(argv[++i]);
		}
		else if (strcmp(argv[i], "--sfpcm") == 0) {
			sampletype = 4;
		}
		else if (strcmp(argv[i], "--ulaw") == 0) {
			sampletype = 5;
		}
		else if (strcmp(argv[i], "--batchi") == 0) {
			batchfiles = list_files(argv[++i], &batchlength);
			dirnm = strip_path_last(argv[i]);
			printf("Batch converting files from \"%s\"\n", dirnm);
		}
		else if (strcmp(argv[i], "-ar") == 0 && i + 1 < argc) {
			resampleoverride = (unsigned int)atoi(argv[++i]);
		}
	}
	mkdir(outpath, 0755);
	ConversionResult rst;

	if (!LoadAAFCLib()) {
		if (batchfiles) free(batchfiles);
		perror("aafc lib load failure");
		return 255;
	}


	if (batchlength == 0) {
		char fnc[256];
		snprintf(fnc, sizeof(fnc), "%s", ofn == NULL || *ofn == '\0' ? filename_without_extension(fn) : filename_without_extension(ofn)); // aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
		char* c = concat_path_ext(outpath, fnc, "aafc");
		if ((rst = convertmedia(fn, c, usemono, normalize, outbps, sampletype, resampleoverride, pitch, nointerp)).statuscode != 0) {
			free(c);
			printf("Failed to convert media: %s [%d]\n", rst.message, rst.statuscode);
			return -128;
		}

		free(c);
	}
	else {
		char dirp[256];
		snprintf(dirp, sizeof(dirp), "%s/%s", outpath, dirnm);

		mkdir(dirp, 0755);

		for (unsigned int i = 0; i < batchlength; i++) {
			char* cf = concat_path_ext(dirp, filename_without_extension(batchfiles[i]), "aafc");
			rst = convertmedia(batchfiles[i], cf, usemono, normalize, outbps, sampletype, resampleoverride, pitch, nointerp);
			if (cf) free(cf);
			if (rst.statuscode == -1)
				continue;
			else if (rst.statuscode < -1) {
				free(batchfiles);
				printf("Failed to convert media: %s [%d]\n", rst.message, rst.statuscode);
				return -128;
			}
			free(batchfiles[i]);
			printf("\r%s %d/%d", "converted", i + 1, batchlength);
		}
		free(batchfiles);
		printf("\r");
	}

	if (rst.statuscode > 0)
		printf("conversion completed with warnings: %s [%d]\n", rst.message, rst.statuscode);
	else printf("%s", "Completed conversion!\n");

	fflush(stdout);
	return 0;
}