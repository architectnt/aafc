/*

	plyr
	(dependency: PortAudio)

	Plays back an AAFC file
	(also an excample in C/C++ how you could use AAFC)

	2024 Architect Enterprises

*/


#include <portaudio.h>
#include <iostream>
#include <aafc.h>
#include <tests.h>

unsigned int sysfreq;

AAFC_HEADER* adata;
float* asmpls;
double ipos = 0;
bool finished = false;
unsigned int totalDurationInSeconds;

static int AudioHandler(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
	float* outspl = (float*)outputBuffer;

	unsigned int smplen = adata->samplelength / adata->channels;

	for (unsigned int i = 0; i < framesPerBuffer; i++) {
		unsigned int inputIndex = (unsigned int)ipos;
		if (inputIndex >= smplen - 1) {
			*(outspl + i * 2) = 0.0f;
			*(outspl + i * 2 + 1) = 0.0f;
			finished = true;
		}
		else {
			double alpha = ipos - inputIndex;
			float spleft = 0.0f, spright = 0.0f;

			if (inputIndex < smplen - 1) {
				if (adata->channels == 1) {
					spleft = spright = (1 - alpha) * asmpls[inputIndex] + alpha * asmpls[inputIndex + 1];
				}
				else if (adata->channels == 2) {
					spleft = (1 - alpha) * asmpls[2 * inputIndex] + alpha * asmpls[2 * (inputIndex + 1)];
					spright = (1 - alpha) * asmpls[2 * inputIndex + 1] + alpha * asmpls[2 * (inputIndex + 1) + 1];
				}
			}

			unsigned int bfri = i * 2;
			*(outspl + bfri) = spleft;
			*(outspl + bfri + 1) = spright;
			ipos += (double)adata->freq / sysfreq;
		}
	}

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
	if(argc > 1) {
		printf("loading AAFC file.. ");

		AAFCOUTPUT aafcfile = ReadFile(argv[1]);
		AAFCDECOUTPUT outp = LoadAAFC(aafcfile.data);
		if (outp.data == NULL) {
			fprintf(stderr, "Failed to load AAFC\n");
			return -1;
		}

		asmpls = outp.data;
		adata = &outp.header;

		totalDurationInSeconds = (adata->samplelength / adata->channels) / adata->freq;
		
		const char* stypeformat;

		switch (adata->sampletype) {
			case 1:
				stypeformat = "PCM";
				break;
			case 2:
				stypeformat = "ADPCM";
				break;
			case 3:
				stypeformat = "DPCM";
				break;
			case 4:
				stypeformat = "SFPCM";
				break;
			case 5:
				stypeformat = "uLaw";
				break;
			default:
				stypeformat = "unformated";
				break;
		}

		printf("Loaded!\n\n-METADATA-\n[Sample Frequency: %d | Channels: %d | Sample Type: %s | AAFC VERSION EXPORTED: AAFC v%d] \n", adata->freq, adata->channels, stypeformat, adata->version);
	}
	else {
		perror("aafc player requires a specifed path argument.");
		return -2;
	}

	Pa_Initialize();
	PaStream* str;
	if (Pa_OpenDefaultStream(&str, 0, 2, paFloat32, 48000, 256, AudioHandler, NULL) != paNoError) {
		perror("aafc player failed to load PortAudio subsystem");
		return -1;
	}

	Pa_StartStream(str);

	const PaStreamInfo* stri = Pa_GetStreamInfo(str);
	sysfreq = (unsigned int)stri->sampleRate;

	while (!finished) {
		drawProgressBar(ipos / adata->freq, totalDurationInSeconds);
		Pa_Sleep(100);
	}

	Pa_StopStream(str);
	Pa_CloseStream(str);
	Pa_Terminate();
	printf("\n");

	return 0;
}
