#Audio Overkill

## What is this?

Audio Overkill started out as a standalone version of the PSF plugin for
audacious.  After making the interface usable and pretty enough, I figured
I could make this into a kinda self-contained video game music player,
similar to Audio Overload.

At the moment, this project is Linux x86 only.  I know the build system is utter
crap, but I've yet to read up on Autoconf (that may take
a while).  All the code I've basically aggregated and modified has only been tested on
x86-64, and some code may be x86 specific (e.g. USF code uses a dynamic recompiler).


Supported formats:

* __PSF__ - Playstation
* __PSF2__ - Playstation 2
* __VGM__ - Various;  Includes Sega Genesis/Mega Drive, PC-88xx, PC-98xx, X68k, PC Engine, and MANY others. 
* __SID__ - Commodore 64
* __SPC__ - SNES/Super Famicom
* __NSF__ - NES/Famicom (including Extended format).
* __SID__ - Atari POKEY
* __AY__  - ZX Spectrum / Amstrad CPC
* __HES__ - NEC Turbografix 16/PC Engine
* __GBS__ - Game Boy
* __GYM__ - Sega Genesis/Mega Drive
* __KSS__ - MSX
* __GSF__ - Game Boy Advance
* __USF__ _ Nintendo 64


What I'm using to build:

* Debian 3.16 amd64
* GCC/G++ 4.9.2
* libsdl2-dev
* libsdl2-ttf-dev
* zlib



## Future

After strengthening the overall framework, I've been able to add new formats,
taking code from other projects with relative ease.

Formats I'd like to add include:

* __DSF__ (Dream Cast)

Additionally, I might add some GUI features like a file selector (GTK+ based perhaps), and
an alternative display showing the queue in a playlist-like format.

Another problem I'm noticing is the amount of disunity in the aggregation of
code.  E.g., the Game Music Emulator and VGMPlay code are each separate multi-format
projects, with their own unifying interface.  These interfaces are in turn attached to
function pointers in Audio Overkill.  Things work generally the way they should, but
inconsitencies in config and tagging just seem jagged and in need to smoothing.  I'm not 
putting my self up to task, as I want to keep this a side-project mostly for personal
amusement.

BTW I want to keep this strictly a video game music player.

## Licensing and Credits

The PSF/PSF2 plugin is uses code from PeOPS  ( /psf/peops* ) written by Pete Bernert, and is licensed under GPL v2.

The VGM code is from [here](https://github.com/vgmrips/vgmplay).  This code is
from Valley Bell, MAME, and numerous other projects which should be listed in /vgm/licenses/List.txt.

(Furthur credits may be found in /vgm/VGMPlay.txt )

The SID plugin is from [here](https://github.com/iPodLinux-Community/TinySID)
and is by T. Hinrichs, R. Sinsch, and Keripo.

Support for the SPC, NSF, AY, KSS, SAP, GBS, GYM, and HES formats are
all courtesy of Blargg's Game Music Emulator (LGPL) .  You can find his stuff [here](http://blargg.8bitalley.com/).

"main.c", "config.c", most of "ao.c", "filelist.c", various "plugin.c" files, and assocated
headers are my code.

As far as I'm aware the original ao.h is from Audio Overload, BSD.

