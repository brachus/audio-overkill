#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#include <dumb.h>

#include "plugin.h"


DUH *duh;
DUH_SIGRENDERER *pduh_sr;



int16_t *pduh_buf[1 * 2 * (44100/60)];

int pduh_depth = 16;
int pduh_unsign = 0;
int pduh_freq = 44100;
int pduh_n_channels = 2;
float pduh_volume = 0.8f;
float pduh_delay = 0.0f;
float pduh_delta;
int pduh_bufsize;

LONG_LONG pduh_length;

int pduh_execute ( void (*update)(const void *, int ))
{
	if (ao_file_open == 1)
	{
		int l = duh_render(pduh_sr, pduh_depth, pduh_unsign, pduh_volume, pduh_delta, pduh_bufsize / pduh_n_channels, pduh_buf);
		
		
		update((uint8_t *) (pduh_buf), l * pduh_n_channels * 2);
	}
	else
		update(0, 0);
	
	return 1;
}

int pduh_open (char * fn)
{
	if (ao_file_open == 1)
		return 0;
	
	int i;
	
	dumb_register_stdfiles();

	dumb_it_max_to_mix = 256;
	
	duh=0;
	
	for (i=0; i<6 && !duh; i++)
	{
		switch(i)
		{
		case 0:duh=load_duh(fn);strcpy(tag_chips, "DUH");break;
		case 1:duh=dumb_load_it(fn);strcpy(tag_chips, "Impulse Tracker");break;
		case 2:duh=dumb_load_xm(fn);strcpy(tag_chips, "FastTracker 2 XM");break;
		case 3:duh=dumb_load_s3m(fn);strcpy(tag_chips, "Scream Tracker S3M");break;
		case 4:duh=dumb_load_mod(fn);strcpy(tag_chips, "Tracker MOD");break;
		case 5:
			play_stat = M_ERR;
			//printf("couldn't open file.\n");
			return 0;
		}
	}
	
	strcpy(tag_system, "D.U.M.B.");
	strcpy(tag_author, "n/a");
	strcpy(tag_game, "n/a");
	
	for (i=0;i<duh->n_tags;i++)
		if (!strcmp(duh->tag[i][0], "TITLE"))
			strcpy(tag_track, duh->tag[i][1]);
	
	pduh_sr = duh_start_sigrenderer(duh, 0, pduh_n_channels, 0);
	
	if (!pduh_sr)
	{
		unload_duh(duh);
		play_stat = M_ERR;
		//printf("couldn't create sigrenderer.\n");
		return 0;
	}
		
	pduh_bufsize = sizeof(int16_t) * 2 * (44100/60);
	
	pduh_length = (LONG_LONG)duh_get_length(duh) * pduh_freq >> 16;
	
	//pduh_buf = (int16_t*) malloc(pduh_bufsize);
	
	pduh_delta = 65536.0f / pduh_freq;
	
	
	ao_file_open = 1;
	
	return 1;
}

void pduh_close ( void )
{
	if (ao_file_open == 0)
		return;
	
	duh_end_sigrenderer(pduh_sr);
	unload_duh(duh);
	
	/*free(pduh_buf);
	pduh_buf=0;*/
	
	ao_file_open = 0;
}

