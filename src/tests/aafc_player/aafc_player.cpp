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

unsigned long sysfreq;
unsigned char syschan;

AAFCDECOUTPUT outp;
double ipos = 0;
bool finished = false;
double totalDurationInSeconds;
unsigned long splen;

static inline double lerp(double a, double b, double t) {
	return a + (b - a) * CLAMP(t, 0.0, 1.0);
}


static int AudioHandler(const void* inp, void* otp, unsigned long frames, const PaStreamCallbackTimeInfo* tinfo, PaStreamCallbackFlags cflags, void* udata) {
	memset(otp, 0, frames * sizeof(float));

	float* outspl = (float*)otp;
	unsigned char ch;
	const double scal = (double)outp.header.freq / sysfreq;
	for (unsigned long i = 0; i < frames; i++) {
		for(ch = 0; ch < syschan; ch++) {
			float smpl = 0;
			if(!finished) {
				double spos = ipos + i * scal;
				if (spos >= splen - 1) {
					ipos = 0;
					finished = true;
				}

				unsigned long index = (unsigned long)spos,
					nxind = (index + 1) % outp.header.samplelength;
				double w = (spos - index);
				
				unsigned char sc = outp.header.channels > 1 ? ch : 0;
				smpl = (float)lerp(
					outp.data[index * outp.header.channels + sc], 
					outp.data[nxind * outp.header.channels + sc], 
					w
				);
			}

			outspl[i * syschan + ch] = smpl;
		}
	}

	if(!finished) ipos += frames * (double)outp.header.freq / sysfreq;

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

	int min = esec / 60,
		sec = (int)esec % 60,
		tmin = tsec / 60,
		tsecs = (int)tsec % 60;

	printf("%02d:%02d/%02d:%02d", min, sec, tmin, tsecs);
	std::cout.flush();
}

int main(int argc, char* argv[]) {
	if(argc > 1) {
		printf("\e[?25l");

		printf("%s", "loading AAFC file.. ");

		AAFCOUTPUT aafcfile = ReadFile(argv[1]);
		outp = LoadAAFC(aafcfile.data);
		if (outp.data == NULL) {
			fprintf(stderr, "Failed to load AAFC\n");
			return -1;
		}

		splen = outp.header.samplelength / outp.header.channels;
		totalDurationInSeconds = ((double)splen) / outp.header.freq;
		
		const char* stypeformat;

		switch (outp.header.sampletype) {
			case 1: stypeformat = "PCM"; break;
			case 2: stypeformat = "ADPCM"; break;
			case 3: stypeformat = "DPCM"; break;
			case 4: stypeformat = "SFPCM"; break;
			case 5: stypeformat = "uLaw"; break;
			default: stypeformat = "unformated"; break;
		}

		printf("Loaded!\n\n-METADATA-\n[Sample Frequency: %lu | Channels: %d | Sample Type: %s | AAFC VERSION EXPORTED: AAFC v%d] \n", outp.header.freq, outp.header.channels, stypeformat, outp.header.version);
		free(aafcfile.data);
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
	if (stri) {
		sysfreq = (unsigned int)stri->sampleRate;
		syschan = 2;
	}
	Pa_StartStream(str);

	while (!finished) {
		drawProgressBar(ipos / outp.header.freq, totalDurationInSeconds);
		Pa_Sleep(100);
	}

	Pa_StopStream(str);
	Pa_CloseStream(str);
	Pa_Terminate();
	printf("%s", "\e[?25h");
	std::cout.flush();
	free(outp.data);
	return 0;
}
