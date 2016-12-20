#ifndef _USF_H_
#define _USF_H_
#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

//#include <libaudcore/plugin.h>
//#include "plugin.h"


#include <glib.h>
#include "cpu.h"
#include "memory.h"

extern int8_t filename[512], title[100];
extern uint32_t cpu_running, use_interpreter, use_audiohle, is_paused,
    cpu_stopped, fake_seek_stopping;
extern uint32_t is_fading, fade_type, fade_time, is_seeking,
    seek_backwards, track_time;
extern double seek_time, play_time, rel_volume;

extern uint32_t enablecompare, enableFIFOfull;



bool usf_init();
int LoadUSF(const char * fn, struct filebuf * fil);
bool usf_play(const char * filename, struct filebuf * file);
//Tuple usf_get_song_tuple(const char * fn, struct filebuf * fil);  TO BE REDONE

#endif
