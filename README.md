# ![aafc_logo](https://architectenterprises.net/cdn/aafc_snwavtr.png) AAFC - Lightweight audio interface
![aafc_logo](https://architectenterprises.net/cdn/aafc_lgo.svg)

**The audio interface designed for ultra-fast loading times**\
..*and a way to bring back audio limitations* ***purposely*** 

## ![aafc_logo](https://architectenterprises.net/cdn/fusionresource/fpg_ico.png) FEATURES
- 1, 4, 8, 12, 16, 24, and 32-bit PCM
- 8-bit uLaw PCM
- 4-bit ADPCM
- Single bit Delta PCM
- 8-bit & 16-bit "Small Float" PCM
- Encoding & Decoding directly
- Resampling
- Normalizer

## Installing
self explanatory git cloning
```
git clone https://github.com/architectnt/aafc.git
```

## Compiling
your average cmake project **:D**\
*(make sure you have cmake installed)*

### WINDOWS - MSVC
This works best with Visual Studio instead of building through CLI as of now.\
``Open a local folder > somewhere where AAFC is located > Select Folder``\
``Build > Install``

### Unix-like
```
mkdir build
cd build
cmake ..
make
```

## AAFC TOOLS
*you may have to enable the `BUILD_TOOLS` option*

### plyr
Small player for debugging audio files\
``./plyr <path-to-aafcfile>``


### aud2aafc
Convert standard audio formats to AAFC

*CLI COMMANDS*\
``-i <path>`` - Input file\
``-ar <newsamplerate>`` - Resample audio to specifed sample rate\
``-p <pitch>`` - Change relative pitch of the audio (1 is normal)\
``-m`` - Force mono\
``--batchi <path>`` - Input folder to batch convert files\
``--adpcm`` - Encode in ADPCM\
``--dpcm`` - Encode in NES-Style Delta PCM\
``--sfpcm`` - Encode in Small Float PCM\
``--ulaw`` - Encode in uLaw\
``--bps`` - Use specific bits per sample

*EXAMPLE*\
``./aud2aafc -i input.wav -m --adpcm -ar 16000``


## FOOTNOTES
Copyright (C) 2024 Architect Enterprises

Architect Audio Clip Format (AAFC) is licenced under the [MIT licence](LICENSE).

AAFC is NOT meant to replace WAV or any PCM related format.