/*

	plyr
	(dependency: PortAudio)

	Plays back an AAFC file
	(also an excample in C/C++ how you could use AAFC)

	2024 Architect Enterprises

*/


#include <portaudio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "aafc.h"
#include "tests.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

int sysfreq;

AAFC_HEADER* adata;
float* asmpls;
double ipos = 0;
bool finished = false;
int totalDurationInSeconds;

typedef float* (*AAFCImport)(const unsigned char*);
typedef AAFC_HEADER* (*AAFCGetHeader)(const unsigned char*);

unsigned char* ReadFile(const char* path) {
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		perror("cant open aafc file :(");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	unsigned char* data = (unsigned char*)malloc(fsize);
	if (data == NULL) {
		perror("MEMORY ALLOC INTERNAL ERROR");
		fclose(file);
		return NULL;
	}

	size_t byrd = fread(data, 1, fsize, file);
	if (byrd != fsize) {
		perror("error trying to load the file");
		fclose(file);
		return NULL;
	}

	fclose(file);
	return data;
}

float* LoadAAFC(const unsigned char* data) {
#ifdef _WIN32
	HMODULE aafcdll = LoadLibrary(LIB_AAFC_RPATH);
	if (aafcdll == NULL) {
		perror("Core AAFC DLL not found!");
		return NULL;
	}

	AAFCImport aimport = (AAFCImport)GetProcAddress(aafcdll, "aafc_import");
	if (aimport == NULL) {
		perror("Could not initialize AAFC functions.");
		return NULL;
	}
	return aimport(data);
#else
	void* hndl = dlopen(LIB_AAFC_RPATH, RTLD_LAZY);
	if (!hndl) {
		fprintf(stderr, "%s\n", dlerror());
		return NULL;
	}
	dlerror();

	AAFCImport aimport = (AAFCImport)dlsym(hndl, "aafc_import");
	char* error;
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		dlclose(hndl);
		return NULL;
	}

	return aimport(data);
#endif
}

AAFC_HEADER* GrabHeader(const unsigned char* data) {
#ifdef _WIN32
	HMODULE aafcdll = LoadLibrary(LIB_AAFC_RPATH);
	if (aafcdll == NULL) {
		perror("Core AAFC DLL not found!");
		AAFC_HEADER* nullhdr{};
		return nullhdr;
	}

	AAFCGetHeader aheader = (AAFCGetHeader)GetProcAddress(aafcdll, "aafc_getheader");
	if (aheader == NULL) {
		perror("Could not initialize AAFC functions.");
		AAFC_HEADER* nullhdr{};
		return nullhdr;
	}
	return aheader(data);
#else
	void* hndl = dlopen(LIB_AAFC_RPATH, RTLD_LAZY);
	if (!hndl) {
		fprintf(stderr, "%s\n", dlerror());
		AAFC_HEADER* nullhdr{};
		return nullhdr;
	}
	dlerror();

	AAFCGetHeader aheader = (AAFCGetHeader)dlsym(hndl, "aafc_getheader");
	char* error;
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		dlclose(hndl);
		AAFC_HEADER* nullhdr{};
		return nullhdr;
	}

	return aheader(data);
#endif
}

static int AudioHandler(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	float* outspl = (float*)outputBuffer;

	const int schannels = 2;

	int channels = adata->channels;
	int smplen = adata->samplelength / channels;
	int samplefreq = adata->freq;

	int i = 0;
	do {
		int inputIndex = (int)ipos;
		if (inputIndex >= smplen - 1) {
			*(outspl + i * 2) = 0.0f;
			*(outspl + i * 2 + 1) = 0.0f;
			finished = true;
		}
		else {
			double alpha = ipos - inputIndex;
			float spleft = 0.0f, spright = 0.0f;

			if (inputIndex < smplen - 1) {
				if (channels == 1) {
					spleft = spright = (1 - alpha) * asmpls[inputIndex] + alpha * asmpls[inputIndex + 1];
				}
				else if (channels == 2) {
					spleft = (1 - alpha) * asmpls[2 * inputIndex] + alpha * asmpls[2 * (inputIndex + 1)];
					spright = (1 - alpha) * asmpls[2 * inputIndex + 1] + alpha * asmpls[2 * (inputIndex + 1) + 1];
				}
			}

			int bfri = i * 2;
			*(outspl + bfri) = spleft;
			*(outspl + bfri + 1) = spright;
			ipos += (double)samplefreq / sysfreq;
		}

		i++;
	} while (i < framesPerBuffer);

	return finished ? paComplete : paContinue;
}

void drawProgressBar(int elapsedSeconds, int totalSeconds) {
	const int barWidth = 30;

	float progress = (float)elapsedSeconds / totalSeconds;
	int pos = barWidth * progress;

	std::cout << "\r";
	std::cout << "[";
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "#";
		else std::cout << " ";
	}
	std::cout << "] ";

	int minutes = elapsedSeconds / 60;
	int seconds = elapsedSeconds % 60;
	int totalMinutes = totalSeconds / 60;
	int totalSecondsRemain = totalSeconds % 60;

	printf("%02d:%02d/%02d:%02d", minutes, seconds, totalMinutes, totalSecondsRemain);
	std::cout.flush();
}

int main(int argc, char* argv[]) {
	if(argc > 1)
	{
		printf("loading AAFC file..\n");

		unsigned char* aafcfile = ReadFile(argv[1]);
		adata = GrabHeader(aafcfile);
		asmpls = LoadAAFC(aafcfile);
		if (asmpls == NULL) {
			fprintf(stderr, "Failed to load AAFC\n");
			return -1;
		}

		totalDurationInSeconds = (adata->samplelength / adata->channels) / adata->freq;
		
		printf("Loaded! Samplerate: %d, Channels: %d\n", adata->freq, adata->channels);
	}
	else {
		perror("aafc player requires a specifed path argument.");
		return -2;
	}

	printf("loading audio subsystem..\n");

	Pa_Initialize();
	PaStream* str;
	PaError err = Pa_OpenDefaultStream(&str,
		0,          // no input channels
		2,          // stereo output
		paFloat32,  // 32 bit floating point output
		48000,      // requested sample rate
		256,        // frames per buffer
		AudioHandler,
		NULL);      // no callback userData
	if (err != paNoError) {
		perror("aafc player failed to load PortAudio subsystem");
	}

	Pa_StartStream(str);

	const PaStreamInfo* stri = Pa_GetStreamInfo(str);
	if (stri) {
		sysfreq = (int)stri->sampleRate;
		printf("AUDIO SUBSYSTEM: using desired sample rate %d\n", sysfreq);
	}

	while (!finished) {
		int elapsed = ipos / adata->freq;

		drawProgressBar(elapsed, totalDurationInSeconds);

		Pa_Sleep(100);
	}

	printf("\n");

	Pa_StopStream(str);
	Pa_CloseStream(str);
	Pa_Terminate();

	return 0;
}
