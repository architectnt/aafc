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
#include "aafc.h"
#include <sndfile.h>
#include <string>
#include <sstream>
#include "tests.h"

typedef struct {
	int statuscode;
	char* message;
} ConversionResult;

ConversionResult convertmedia(const char* fn, const char* outpath, bool usemono, bool normalize, unsigned char bps, unsigned char sampletype, unsigned int spoverride, float pitch) {
	ConversionResult s = { 0, NULL};

	SF_INFO info;
	memset(&info, 0, sizeof(SF_INFO));

	SNDFILE* ifl = sf_open(fn, SFM_READ, &info);
	if (!ifl) {
		s.message = (char*)"failed to open file :("; // what
		s.statuscode = -1;
		return s;
	}

	size_t nitms = info.frames * info.channels;
	float* smpl = (float*)malloc(nitms * sizeof(float));
	if (smpl == NULL) {
		sf_close(ifl);
		s.message = (char*)"failed to allocate memory D:"; // what
		s.statuscode = -2;
		return s;
	}

	if (sf_readf_float(ifl, smpl, info.frames) != info.frames) {
		s.message = (char*)"result has an unmatched sample count"; // what
		s.statuscode = 1;
	}

	AAFCOUTPUT out = ExportAAFC(smpl, info.samplerate, info.channels, nitms, bps, sampletype, usemono, spoverride, normalize, pitch);
	if (out.data == NULL) {
		sf_close(ifl);
		free(smpl);
		s.message = (char*)"EXPORT FAILED >:("; // what
		s.statuscode = -3;
		return s;
	}

	FILE* ofile = fopen(outpath, "wb");
	if (ofile == NULL) {
		sf_close(ifl);
		free(smpl);
		s.message = (char*)"could not open file >:((((((((((("; // what
		s.statuscode = -4;
		return s;
	}

	fwrite(out.data, sizeof(unsigned char), out.size, ofile);

	sf_close(ifl);
	free(smpl);

	fclose(ofile);
	return s;
}

int main(int argc, char* argv[]) {
	const char* fn = "input.wav";
	unsigned char outbps = 16;
	bool usemono = false;
	bool normalize = false;
	unsigned int batchlength = 0;
	char** batchfiles;
	const char* dirnm;
	unsigned char sampletype = 1;
	unsigned int resampleoverride = 0;
	float pitch = 1;

	for (unsigned int i = 1; i < argc; i++) {
		std::string input = std::string(argv[i]);
		if (input == "-i" && i + 1 < argc) {
			fn = argv[++i];
		}
		else if (input == "--bps" && i + 1 < argc) {
			outbps = (unsigned int)std::stof(argv[++i], NULL);
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
		else if (input == "--ulaw") {
			sampletype = 5;
		}
		else if (input == "--batchi") {
			batchfiles = list_files(argv[++i], &batchlength);
			dirnm = strip_path_last(argv[i]);
			printf("Batch converting files from \"%s\"\n", dirnm);
		}
		else if (input == "-ar" && i + 1 < argc) {
			resampleoverride = (unsigned int)std::stof(argv[++i], NULL);
		}
	}
	mkdir("aafc_conversions", 0755);
	ConversionResult rst;

	if (batchlength == 0) {
		char* c = concat_path("aafc_conversions/", filename_without_extension(fn));
		if ((rst = convertmedia(fn, c, usemono, normalize, outbps, sampletype, resampleoverride, pitch)).statuscode != 0) {
			free(c);
			printf("Failed to convert media: %s [%d]\n", rst.message, rst.statuscode);
			return -128;
		}

		free(c);
	}
	else {
		char dirp[256];
		snprintf(dirp, sizeof(dirp), "aafc_conversions/%s", dirnm);

		mkdir(dirp, 0755);

		for (unsigned int i = 0; i < batchlength; i++) {
			rst = convertmedia(batchfiles[i], concat_path(dirp, filename_without_extension(batchfiles[i])), usemono, normalize, outbps, sampletype, resampleoverride, pitch);
			if (rst.statuscode == -1) {
				continue;
			}
			else if (rst.statuscode < -1) {
				free(batchfiles);
				printf("Failed to convert media: %s [%d]\n", rst.message, rst.statuscode);
				return -128;
			}
			free(batchfiles[i]);
		}
		free(batchfiles);
	}

	if (rst.statuscode > 0) {
		printf("conversion completed with warnings: %s [%d]\n", rst.message, rst.statuscode);
	}
	else {
		printf("Completed conversion!\n");
	}

	return 0;
}