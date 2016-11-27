

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../ao.h"

#include "gme.h"

#include "plugin.h"


extern int ao_track_select;

Music_Emu* emu;

short *wave_buf;

char subsong_disp[32];



void gme_fill_tags(struct track_info_t *tinfo, const char* fn)
{
	
	if (tinfo->song[0]!='\0')
		strcpy(tag_track, tinfo->song);
		
	if (tinfo->author[0]!='\0')
		strcpy(tag_author, tinfo->author);
		
	if (tinfo->system[0]!='\0')
		strcpy(tag_system, tinfo->system);
		
	if (tinfo->game[0]!='\0')
		strcpy(tag_game, tinfo->game);
	
	strcpy(tag_chips, "???");
}

int gme_execute ( void (*update)(const void *, int ))
{
	if (gme_play( emu, SAMPLERATE, wave_buf ) != 0)
		update(0, 0);
	
	else
		update((u_int8_t * ) wave_buf, sizeof(short) * SAMPLERATE);
	
	return 1;
		
}

int gme_open ( char * fn)
{
	struct track_info_t track_info;
	int tmp;
	
	/* open file with new emulator */
	
	if (gme_open_file( fn, &emu, (long) ao_sample_rate ) != 0)
		return 0;
	
	/* find out max tracks for a given file */
	
	if (gme_track_info( emu, &track_info, 0 ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
		
	if (ao_track_select >= track_info.track_count)
		ao_track_select = track_info.track_count-1;
	
	if (ao_track_select < 0)
		ao_track_select = 0;
	
	
	if (gme_track_info( emu, &track_info, ao_track_select ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
	
	gme_fill_tags(&track_info, fn);
	
	
	/* watch this */
	if (track_info.track_count > 1)
	{
		sprintf(subsong_disp," (%d/%d)",ao_track_select+1,track_info.track_count);
		strcat(tag_track, subsong_disp);
	}
	
	if ( gme_start_track( emu, ao_track_select ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
	

	
	
	wave_buf = (short *) malloc (sizeof(short) * SAMPLERATE);
	
	
	return 1;
}

void gme_close ( void )
{
	gme_delete( emu );
	
	free(wave_buf);
}

