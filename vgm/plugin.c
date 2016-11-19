#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>


#include "plugin.h"

#include "../ao.h"

#include "chips/mamedef.h"
#include "stdbool.h"
#include "VGMPlay.h"
#include "VGMPlay_Intf.h"

#include <iconv.h>




#define SAMPLESIZE sizeof(WAVE_16BS)


#define SAMPLERATE 2048

UINT8 CmdList[0x100]; // used by VGMPlay.c and VGMPlay_AddFmts.c
bool ErrorHappened;   // used by VGMPlay.c and VGMPlay_AddFmts.c
extern VGM_HEADER VGMHead;
extern UINT32 SampleRate;
extern UINT32 VGMMaxLoopM;
extern UINT32 VGMMaxLoop;
extern GD3_TAG VGMTag;
extern UINT32 FadeTime;
extern bool EndPlay;



static int preferJapTag = 0;

WAVE_16BS *wave_16bs_buf;

static int buffered_len;


wchar_t * select_tagje(wchar_t *e, wchar_t *j, int preferj)
{
	if (preferj)
	{
		if (!j)
		{
			if (!e)
				return 0;
			return e;
		}
		return j;
	}
	else
	{
		if (!e)
		{
			if (!j)
				return 0;
			return j;
		}
		return e;
	}
	return 0;
}

void fill_tags()
{
	iconv_t ico;
	int tmp;
	
	wchar_t *track_name = 0, *author_name = 0, *game_name = 0;
	
	track_name =
		select_tagje(VGMTag.strTrackNameE, VGMTag.strTrackNameJ, preferJapTag);
	author_name =
		select_tagje(VGMTag.strAuthorNameE, VGMTag.strAuthorNameJ, preferJapTag);
	game_name =
		select_tagje(VGMTag.strGameNameE, VGMTag.strGameNameJ, preferJapTag);
	
	safe_strcpy(tag_track, "???", 256);
	
	//printf("track_len.length = %d\n",wcslen(track_name));
	
	if (track_name!=0)
	{
		tmp = wcstombs(tag_track,track_name,256);
		if (tmp <=0)
			safe_strcpy(tag_track, "???", 256);
	}
	if (author_name!=0)
	{
		tmp = wcstombs(tag_author,author_name,256);
		if (tmp <=0)
			safe_strcpy(tag_author, "???", 256);
	}
	if (game_name!=0)
	{
		tmp = wcstombs(tag_game,game_name,256);
		if (tmp <=0)
			safe_strcpy(tag_game, "???", 256);
	}
	
		
}

int vgm_execute ( void (*update)(const void *, int ))
{
	/* create a buffer consisting of u8 ints, and use it to satisfy update.*/
	
	
	buffered_len = FillBuffer(wave_16bs_buf, SAMPLERATE);
	
	
	if (buffered_len) /* remember that arg 2 must be size in bytes */
		update((UINT8 * ) wave_16bs_buf, SAMPLESIZE * buffered_len);
	else
		update(0, 0);
	
	return 1;
	
}

int vgm_open ( char * fn)
{
	
	
	
	VGMPlay_Init();
	
	/* loop forever */
	VGMMaxLoop = 0x00;
	
	
	
	
    VGMPlay_Init2();
    
    
    clear_tags();
    
	if ( !OpenVGMFile(fn) )
	{
		play_stat = M_ERR;
		
		return 0;
	}
	
	/* prefer english for now. */
	
	fill_tags();
	
	
	/*
	safe_strcpy(tag_track, (char *) VGMTag.strTrackNameE, 256);
	safe_strcpy(tag_author, (char *) VGMTag.strAuthorNameE, 256);
	safe_strcpy(tag_game, (char *) VGMTag.strGameNameE, 256);*/
	

	PlayVGM();
	
	
	wave_16bs_buf = (WAVE_16BS *) malloc (SAMPLESIZE * SAMPLERATE);
		
	
	return 1;
}

void vgm_close ( void (*update)(const void *, int ))
{
	free(wave_16bs_buf);
	
	StopVGM();
    CloseVGMFile();
    VGMPlay_Deinit();
}
