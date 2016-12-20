#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>






extern "C"
{
	#include "../ao.h"
	
	#include "../conf.h"
	
	#include "plugin.h"
}



#include "types.h"

#include "VBA/GBA.h"


int cpupercent=0, sndSamplesPerSec, sndNumChannels;
int sndBitsPerSample=16;

int deflen=120,deffade=4;

extern unsigned short soundFinalWave[1470];
extern int soundBufferLen;

extern char soundEcho;
extern char soundLowPass;
extern char soundReverse;
extern char soundQuality;

double decode_pos_ms; // current decoding position, in milliseconds
int seek_needed; // if != -1, it is the point that the decode thread should seek to, in ms.

static int g_playing = 0;
static int g_must_exit = 0;

extern "C" int LengthFromString(const char * timestring);
extern "C" int VolumeFromString(const char * volumestring);

extern "C"
{
	#include "VBA/psftag.h"
	#include "gsf.h"
	
	#include "VBA/GBA.h"
}




extern "C" { 
int defvolume=1000;
int relvolume=1000;
int TrackLength=0;
int FadeLength=0;
int IgnoreTrackLength, DefaultLength=150000;
int playforever=1;
int fileoutput=0;
int TrailingSilence=1000;
int DetectSilence=0, silencedetected=0, silencelength=5;

void (*tmp_update )(const void *, int);

}

extern "C" void end_of_track()
{
	g_playing = 0;
}
	
extern "C" void writeSound(void)
{
	//printf("write sound: %d %d\n",soundBufferLen,tmp_update);
	
	if (soundBufferLen)
		tmp_update((u_int8_t *) soundFinalWave, soundBufferLen);
	
	
	
	decode_pos_ms +=
		(soundBufferLen/(2*sndNumChannels) * 1000)
			/ (float)sndSamplesPerSec;
}

extern "C" void signal_handler(int sig)
{
	struct timeval tv_now;
	int elaps_milli;
	
	static int first=1;
	static struct timeval last_int = {0,0};
	
	g_playing = 0;
	gettimeofday(&tv_now, NULL);

	if (first) {
		first = 0;
	}
	else {
		elaps_milli = (tv_now.tv_sec - last_int.tv_sec)*1000;
		elaps_milli += (tv_now.tv_usec - last_int.tv_usec)/1000;

		if (elaps_milli < 1500) {
			g_must_exit = 1;
		}
	}
	memcpy(&last_int, &tv_now, sizeof(struct timeval));
}


#define EMU_COUNT 250000


int gsf_execute ( void (*update)(const void *, int ))
{
	
	tmp_update = update;
		
	emu_loop();
}

void gsf_load_conf(const char *fn);

int gsf_open (char * fn)
{
	int r, tmp;
	char Buffer[1024];
	char length_str[256], fade_str[256], volume[256], title_str[256];
	char tmp_str[256];
	char *tag;

	soundLowPass = 0;
	soundEcho = 0;
	soundQuality = 1; /* must be set to 1 or infinite loop will happen */

	DetectSilence=0;
	silencelength=0;	
	IgnoreTrackLength=1;
	DefaultLength=150000;
	TrailingSilence=1000;
	playforever=1;
	
	gsf_load_conf("./ao.conf");
	
	if (!GSFRun(fn))
		return 0;
	
	tag = (char*) malloc(50001);
	
	psftag_readfromfile((void*) tag, fn);
	
	
	clear_tags();
	
	if (!psftag_getvar(tag, "title", tmp_str, sizeof(tmp_str)-1))
		strcpy(tag_track, tmp_str);
	if (!psftag_getvar(tag, "artist", tmp_str, sizeof(tmp_str)-1))
		strcpy(tag_author, tmp_str);
	if (!psftag_getvar(tag, "game", tmp_str, sizeof(tmp_str)-1))
		strcpy(tag_game, tmp_str);
		
	if (!psftag_getvar(tag, "year", tmp_str, sizeof(tmp_str)-1))
		strcpy(tag_year, tmp_str);
		
	strcpy(tag_system, "Game Boy Advance");
		
	if (!psftag_getvar(tag, "fade", fade_str, sizeof(fade_str)-1))
		FadeLength = LengthFromString(fade_str);
		
	if (!psftag_raw_getvar(tag, "length", length_str, sizeof(length_str)-1))
	{
		TrackLength = LengthFromString(length_str) + FadeLength;
		
		if (IgnoreTrackLength)
			TrackLength = DefaultLength;
	}
	else
		TrackLength = DefaultLength;
	
	free(tag);
	
	emuticksleft = EMU_COUNT; /* important */
	emuhold=0;
	
	
	
	
}

void gsf_close ( void )
{
	GSFClose();
	
}


void gsf_load_conf(const char *fn)
{
	
	struct cfg_entry *o, *tmp;
	
	
	o = read_conf(fn);
	
	if (!o)
		return;
	
	tmp = o;
	
	
	#ifndef CONF_IS_BOOL
	 #define CONF_IS_BOOL (tmp->type==E_BOOL || tmp->type==E_INT)
	 #define ENTRY_NAME(x) (strcmp(x,tmp->name) == 0)
	 #define CONF_INT (tmp->dat[0].i)
	 #define CONF_FLOAT (tmp->dat[0].f)
	#endif
	
	while (tmp)
	{
		
		
		if (!strcmp("Game Boy Advance", tmp->section) || 
			!strcmp("GBA", tmp->section))
		{
			if (ENTRY_NAME("enable_echo")  && CONF_IS_BOOL)
				soundEcho = CONF_INT;
			if (ENTRY_NAME("ignore_track_length")  && CONF_IS_BOOL)
				IgnoreTrackLength = CONF_INT;
			if (ENTRY_NAME("play_forever")  && CONF_IS_BOOL)
				playforever = CONF_INT;
			if (ENTRY_NAME("relvolume")  && tmp->type==E_INT)
				relvolume = CONF_INT;
			if (ENTRY_NAME("defvolume")  && tmp->type==E_INT)
				defvolume = CONF_INT;
			
		}
		
		tmp = tmp->next;
	}
			

}
