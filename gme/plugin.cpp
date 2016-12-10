

#include <stdlib.h>
#include <string.h>
#include <stdio.h>





extern "C"
{
	#include "../ao.h"
	#include "../conf.h"
};

#include "gme.h"

#include "plugin.h"


extern int ao_track_select, ao_track_max;

Music_Emu* emu;

short *wave_buf;

char subsong_disp[32];

void gme_load_conf (const char * fn);



void gme_fill_tags(struct track_info_t *tinfo, const char* fn)
{
	char *ftmp;
	if (tinfo->song[0]!='\0')
		strcpy(tag_track, tinfo->song);
	else
	{
		ftmp = strip_fn(fn);
		if (ftmp)
		{
			strcpy(tag_track, ftmp);
			free(ftmp);
		}
		
	}
		
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
	
	if (ao_file_open == 1)
		return 0;
	
	struct track_info_t track_info;
	int tmp;
	
	gme_load_conf ( "./ao.conf" );
	
	
	/* open file with new emulator */
	
	if (gme_open_file( fn, &emu, (long) ao_sample_rate ) != 0)
		return 0;
	
	/* find out max tracks for a given file */
	
	if (gme_track_info( emu, &track_info, 0 ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
	
	ao_track_max = track_info.track_count - 1;
	if (ao_track_select > ao_track_max)
		ao_track_select = ao_track_max;
	
	if (ao_track_select < 0)
		ao_track_select = 0;
	
	if (gme_track_info( emu, &track_info, ao_track_select ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
	
	gme_fill_tags(&track_info, fn);
	
	if (ao_track_max == 0)
		ao_track_max = -1;
	
		
	if ( gme_start_track( emu, ao_track_select ) != 0)
	{
		gme_delete( emu );
		return 0;
	}
	
	
	
	wave_buf = (short *) malloc (sizeof(short) * SAMPLERATE);
	
	if ( wave_buf == 0)
		return 0;
	
	ao_file_open = 1;
	
	return 1;
}

void gme_close ( void )
{
	if (ao_file_open == 0)
		return;
	
	gme_delete( emu );
	
	free(wave_buf);
	
	ao_file_open = 0;
}



void gme_load_conf (const char * fn)

{
	struct cfg_entry *o, *tmp;
	
	o = read_conf(fn);
	
	if (!o)
		return;
	
	tmp = o;
	
	#if DEBUG_MAIN
	print_cfg_entries(o);
	#endif
	
	#ifndef CONF_IS_BOOL
	 #define CONF_IS_BOOL (tmp->type==E_BOOL || tmp->type==E_INT)
	 #define ENTRY_NAME(x) (strcmp(x,tmp->name) == 0)
	 #define CONF_INT (tmp->dat[0].i)
	 #define CONF_FLOAT (tmp->dat[0].f)
	#endif
	
	while (tmp)
	{
		if (!strcmp("SPC",tmp->section))
		{
			if (ENTRY_NAME("enable_echo") && CONF_IS_BOOL)
				ao_set_spc_echo = tmp->dat[0].i;
		}
		
		tmp = tmp->next;
	}
	
	
	if (o!=0)
		free_cfg_entries(o);

}
