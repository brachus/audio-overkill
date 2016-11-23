
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "../ao.h"



#include "plugin.h"


static word init_addr, play_addr;
static byte actual_subsong, max_subsong, play_speed;
static char song_name[32], song_author[32], song_copyright[32], subsong_disp[32];

static word *wave_buf;

/* get rid of sid_subsong_sel */
extern int sid_subsong_sel;


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
	
	synth_render(wave_buf, SAMPLERATE);
	
	
	if (buf_len) /* remember that arg 2 must be size in bytes */
		update((u_int8_t * ) wave_buf, sizeof(word) * SAMPLERATE);
	else
		update(0, 0);
		
	
	return 1;
	
}


int sid_open (char * fn)
{
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
	
	/* get rid of sid_subsong_sel */	
	sid_subsong_sel = (sid_subsong_sel > max_subsong) ?
		max_subsong : sid_subsong_sel;
	sid_subsong_sel = (sid_subsong_sel<0)? 0: sid_subsong_sel;

	/* watch this */
	if (max_subsong>0)
	{
		sprintf(subsong_disp," (%d/%d)",sid_subsong_sel+1,max_subsong+1);
		strcat(tag_track, subsong_disp);
	}
	

	cpuJSR(init_addr, sid_subsong_sel);
	
	
	wave_buf = (word *) malloc (sizeof(word) * SAMPLERATE);
	
		
	return 1;
}

void sid_close ( void )
{
	free(wave_buf);
}
