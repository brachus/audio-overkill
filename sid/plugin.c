
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "../ao.h"


#include "plugin.h"


extern word init_addr, play_addr;
extern byte actual_subsong, max_subsong, play_speed;
extern char song_name[32], song_author[32], song_copyright[32], subsong_disp[32];

extern word *wave_buf;

/* get rid of sid_subsong_sel */
extern int ao_track_select, ao_track_max;


void sid_fill_tags()
{
	
	if (song_name[0]!='\0')
		strcpy(tag_track, song_name);
		
	if (song_author[0]!='\0')
		strcpy(tag_author, song_author);
	
	if (song_copyright[0]!='\0')
		strcpy(tag_game, song_copyright);	
	
	strcpy(tag_system, "Commodore 64");
	strcpy(tag_chips, "MOS 6581/8580 SID");
}



int sid_execute ( void (*update)(const void *, int ))
{
	int buf_len=1;
	
	/* create a buffer consisting of u8 ints, and use it to satisfy update.*/
	
	cpuJSR(play_addr, 0);
	
	synth_render(wave_buf, SID_SAMPLERATE);
	
	
	if (buf_len) /* remember that arg 2 must be size in bytes */
		update((u_int8_t * ) wave_buf, sizeof(word) * SID_SAMPLERATE);
	else
		update(0, 0);
		
	
	return 1;
	
}


int sid_open (char * fn)
{
	if (ao_file_open == 1)
		return;
	
    clear_tags();
        
    c64Init();
	synth_init(44100);
    
    /* here */
    if (! c64SidLoad(
			fn,
			&init_addr,
			&play_addr,
			&actual_subsong, 
			&max_subsong,
			&play_speed,
			song_name,
			song_author,
			song_copyright) )
	{
		play_stat = M_ERR;
		return 0;
	}
		
	setlocale(LC_CTYPE, "");
	
	sid_fill_tags();
	
	ao_track_max = max_subsong;
	
	/* get rid of sid_subsong_sel */
	if (ao_track_select > ao_track_max)
		ao_track_select = ao_track_max;
	
	if (ao_track_select < 0)
		ao_track_select = 0;

	
	if (ao_track_max == 0)
		ao_track_max = -1;
		
	

	cpuJSR(init_addr, ao_track_select);
	
	
	wave_buf = (word *) malloc (sizeof(word) * SID_SAMPLERATE);
	
	ao_file_open = 1;
		
	return 1;
}

void sid_close ( void )
{
	if (ao_file_open == 0)
		return;
	
	free(wave_buf);
	
	ao_file_open = 0;
}
