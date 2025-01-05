# ![aafc_logo](https://architectenterprises.net/cdn/aafc_snwavtr.png) AAFC - Lightweight audio interface
![aafc_slogo](https://architectenterprises.net/cdn/aafc_lgo.svg)

**The audio interface designed for ultra-fast loading times and runtime importing on-the-fly**\
..*and a way to bring back audio limitations* ***purposely*** 

## ![aafc_logo](https://architectenterprises.net/cdn/fusionresource/fpg_ico.png) FEATURE HIGHLIGHT

### EXPORT/IMPORT
| TYPE | SUPPORTED BIT DEPTHS |
| ------------- | ------------- |
| PCM | `1` `3` `4` `8` `10` `12` `16` `24` `32 (float)` |
| SFPCM | `8` `16` |
| ADPCM | `4` |
| DPCM | `1` |
| ULAW | `8` |

### MODIFIERS
- Resampling w/ pitch shifting
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
*you may have to enable the `BUILD_TOOLS` option additionally with `BUILD_SHARED_LIBS`*

### plyr
Small player for debugging audio files\
``./plyr <path-to-aafcfile>``


### aud2aafc
Convert standard audio formats to AAFC


*CLI COMMANDS*
| ARGUMENT | PARAMETER | DESCRIPTION |
| ------------- | ------------- | ------------- |
| `-i` | `relative/absolute path` | Input file |
| `--batchi` | `relative/absolute path` | Input folder to batch convert files |
| `--bps` | `whole number` | Use specific bits per sample |
| `-ar` | `whole number` | Resample audio to specified sample rate |
| `-p` | `number (1.0 is normal)` | Change relative pitch of the audio |
| `-m` | - | Force mono |
| `-n` | - | Normalize |
| `-o` | `relative/absolute path` | Output directory |
| `-fn` | `name` | Output filename |
| `--adpcm` | - | Encode in ADPCM |
| `--dpcm` | - | Encode in Delta PCM |
| `--sfpcm` | - | Encode in 'Small Float' PCM |
| `--ulaw` | - | Encode in uLaw |

*EXAMPLE*\
``./aud2aafc -i input.wav -m --adpcm -ar 16000``


## FOOTNOTES
Copyright (C) 2024-2025 Architect Enterprises

Architect Audio Clip Format (AAFC) is licenced under the [MIT licence](LICENSE).

AAFC is NOT meant to replace WAV or any PCM related format.