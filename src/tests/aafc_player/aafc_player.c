/*

	plyr
	(dependency: PortAudio)

	Plays back an AAFC file
	(also an example in C/C++ how you could use AAFC for playback)

	2024-2025 Architect Enterprises

*/

#include <portaudio.h>
#include <stdio.h>
#include "../../aafc/aafc.h"
#include "../fileutils.h"
#include "../libaafcfunc.h"

unsigned int sysfreq;
unsigned char syschan;

AAFCDECOUTPUT outp;
double ipos = 0;
unsigned char finished = 0;
unsigned int splen;

static int AudioHandler(const void* inp, void* otp, unsigned long frames, const PaStreamCallbackTimeInfo* tinfo, PaStreamCallbackFlags cflags, void* udata) {
	memset(otp, 0, frames * sizeof(float));

	float* outspl = (float*)otp;
	unsigned char ch;
	const double scal = (double)outp.header.freq / sysfreq;
	for (unsigned int i = 0; i < frames; i++) {
		for(ch = 0; ch < syschan; ch++) {
			float smpl = 0;
			if(!finished) {
				if (ipos >= splen - 1) { finished = 1; ipos = 0; }

				unsigned int idx = (unsigned int)ipos, 
					nx = (idx + 1) % splen;
				double frac = ipos - idx;

				unsigned char sc = outp.header.channels > 1 ? ch : 0;
				float s0 = outp.data[idx * outp.header.channels + sc],
					s1 = outp.data[nx * outp.header.channels + sc];
				smpl = (float)(s0 + (s1 - s0) * frac);
			}
			outspl[i * syschan + ch] = smpl;
		}
		ipos += scal;
	}
	return finished ? paComplete : paContinue;
}

static void drawProgressBar() {
	const unsigned char barWidth = 32;
	unsigned char pos = barWidth * (ipos / (float)splen);
	unsigned int tl = splen / outp.header.freq, 
		el = ((int)ipos) / outp.header.freq;

	printf("%s", "\r[");
	for (unsigned char i = 0; i++ < barWidth;)
		printf("%s", ((i <= pos) ? "=" : " "));
	printf("%s", "] ");

	printf("%02d:%02d / %02d:%02d", 
		el / 60, // elapsed m
		el % 60, // s
		tl / 60, // total m
		tl % 60 // s
	);
}

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		perror("aafc player requires a specifed path argument.");
		return -2;
	}

	if (!LoadAAFCLib()) {
		perror("aafc lib load failure");
		return 255;
	}
	printf("%s", "loading AAFC file.. ");

	AAFCOUTPUT aafcfile = ReadAAFCFile(argv[1]);
	if ((outp = LoadAAFC(aafcfile.data)).header.signature == 0) {
		fprintf(stderr, "Failed to load AAFC\n");
		return -1;
	}
	free(aafcfile.data);

	splen = outp.header.samplelength / outp.header.channels;

	const char* stypeformat;
	switch (outp.header.sampletype) {
		case 1: stypeformat = "PCM"; break;
		case 2: stypeformat = "ADPCM"; break;
		case 3: stypeformat = "DPCM"; break;
		case 4: stypeformat = "SFPCM"; break;
		case 5: stypeformat = "uLaw"; break;
		default: stypeformat = "unformated"; break;
	}

	printf("\r-AAFC (clip version v%d) METADATA-\n[Sample Rate: %d | Channels: %d | Sample Type: %s @ %d bps] \n", outp.header.version, outp.header.freq, outp.header.channels, stypeformat, outp.header.bps);

	PaStream* str;
	Pa_Initialize();
	if (Pa_OpenDefaultStream(&str, 0, 2, paFloat32, 48000, 256, AudioHandler, NULL) != paNoError) {
		perror("aafc player failed to load PortAudio subsystem");
		return -1;
	}

	const PaStreamInfo* stri = Pa_GetStreamInfo(str);
	if (stri) {
		sysfreq = (unsigned int)stri->sampleRate;
		syschan = 2;
	}

	Pa_StartStream(str);

	printf("\e[?25l"); // hide cursor here because why see it traverse everywhere for no reason
	while (!finished) {
		drawProgressBar();
		Pa_Sleep(100);
	}

	Pa_StopStream(str);
	Pa_CloseStream(str);
	Pa_Terminate();
	printf("%s", "\e[?25l\n");
	free(outp.data);
	return 0;
}
