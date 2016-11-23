#Audio Overkill

## What is this?

Audio Overkill started out as a standalone version of the PSF plugin for
audacious.  After making the interface usable and pretty enough, I figured
I could make this into a kinda self-contained video game music player,
similar to Audio Overload.

At the moment, this project is Linux only.  I know the build system is utter
crap, but I've yet to read up on Autoconf (being still in college, that may take
a while).

Supported formats:

* __PSF__ - Playstation.  (hadn't got around to PSF2 yet.)
* __VGM__ - Various;  Primarily Sega Genesis/Mega Drive and Game Gear
		but MANY others as well.
* __SID__ - Commodore 64.



## Future

After strengthening the overall framework, I've been able to add new formats,
taking code from other projects with relative ease.

Formats coming soon include:

* __USF__ (N64)
* __NSF__ (NES/Famicom)
* __PSF2__ (Playstation 2)

Additionally, I might add some GUI features like a file selector (GTK+ based perhaps), and
an alternative display showing the queue in a playlist-like format.

I want to keep this strictly a video game music player.

## Licensing and Credits

The PSF/PSF2 plugin is uses code from PeOPS  ( /psf/peops* ) written by Pete Bernert, and is licensed under GPL v2.

The VGM code is from [here](https://github.com/vgmrips/vgmplay).  This code is
from Valley Bell, MAME, and numerous other projects which should be listed in /vgm/licenses/List.txt.

(Furthur credits may be found in /vgm/VGMPlay.txt )

The SID plugin is from [here](https://github.com/iPodLinux-Community/TinySID)
and is by T. Hinrichs, R. Sinsch, and Keripo.

"main.c", "config.c", most of "ao.c", "filelist.c", various "plugin.c" files, and assocated
headers are my code.

As far as I'm aware the original ao.h is from Audio Overload, BSD.

