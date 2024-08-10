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
unsigned char syschan;

AAFC_HEADER* adata;
float* asmpls;
double ipos = 0;
bool finished = false;
double totalDurationInSeconds;
unsigned int splen;


// noooooo
static inline double mind(double a, double b) {
	return (a < b) ? a : b;
}

static inline double maxd(double a, double b) {
	return (a > b) ? a : b;
}

static inline double clampd(double val, double a, double b) {
	return maxf(minf(val, b), a);
}

static inline double lerp(double a, double b, double t) {
	return a + (b - a) * clampd(t, 0.0f, 1.0f);
}

static int AudioHandler(const void* inp, void* outp, unsigned long frames, const PaStreamCallbackTimeInfo* tinfo, PaStreamCallbackFlags cflags, void* udata) {
	memset(outp, 0, frames * sizeof(float));

	float* outspl = (float*)outp;
	unsigned char ch;
	for (unsigned int i = 0; i < frames; i++) {
		for(ch = 0; ch < syschan; ch++) {
			float smpl = 0;
			if(!finished) {
				double spos = ipos + i * ((double)adata->freq / sysfreq);
				if (spos >= splen - 1) {
					ipos = 0;
					finished = true;
				}

				unsigned int index = (unsigned int)spos;
				unsigned int nxind = (index + 1) % adata->samplelength;
				double w = (spos - index);
				
				unsigned char sc = adata->channels > 1 ? ch : 0;
				smpl = (float)lerp(*(asmpls + (index * adata->channels + sc)), *(asmpls + (nxind * adata->channels + sc)), w);
			}

			*(outspl + i * syschan + ch) = smpl;
		}
	}

	if(!finished) ipos += frames * (double)adata->freq / sysfreq;

	return finished ? paComplete : paContinue;
}

static void drawProgressBar(double esec, double tsec) {
	const unsigned int barWidth = 32;

	double progress = esec / tsec;
	int pos = barWidth * progress;

	std::cout << "\r";
	std::cout << "[";
	for (unsigned int i = 0; i < barWidth; ++i) {
		std::cout << ((i <= pos) ? "=" : " ");
	}
	std::cout << "] ";

	int min = esec / 60;
	int sec = (int)esec % 60;
	int tmin = tsec / 60;
	int tsecs = (int)tsec % 60;

	printf("%02d:%02d/%02d:%02d", min, sec, tmin, tsecs);
	std::cout.flush();
}

int main(int argc, char* argv[]) {
	if(argc > 1) {
		printf("\e[?25l");

		printf("loading AAFC file.. ");

		AAFCOUTPUT aafcfile = ReadFile(argv[1]);
		AAFCDECOUTPUT outp = LoadAAFC(aafcfile.data);
		if (outp.data == NULL) {
			fprintf(stderr, "Failed to load AAFC\n");
			return -1;
		}

		asmpls = outp.data;
		adata = &outp.header;

		splen = adata->samplelength / adata->channels;
		totalDurationInSeconds = ((double)splen) / adata->freq;
		
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

	const PaStreamInfo* stri = Pa_GetStreamInfo(str);
	sysfreq = (unsigned int)stri->sampleRate;
	syschan = (unsigned char)2;
	Pa_StartStream(str);

	while (!finished) {
		drawProgressBar(ipos / adata->freq, totalDurationInSeconds);
		Pa_Sleep(100);
	}

	Pa_StopStream(str);
	Pa_CloseStream(str);
	Pa_Terminate();
	printf("\e[?25h");
	std::cout.flush();

	return 0;
}
