
#include "plugin.h"

int load_psf_file(char *fn)
{
	if (ao_file_open == 1)
		return 0;
	
	
	char *ctmp;
	struct filebuf *fb = filebuf_init();
	int tlen, fok;
	FILE *fp;
	
	clear_tags();
	
	u_int8_t *tbuf;
	
	
	if (fn != 0)
	{
		ctmp = strip_dir(fn);
		
		
		ao_set_lib_dir(ctmp);
		
	}
	
	filebuf_load(fn, fb);
	
	if (fb->len==0)
	{
		play_stat = M_ERR;
		
		return 0;
	}
	
	
	fok = psf_start(fb->buf, fb->len);
	
	if (fok == AO_FAIL)
		play_stat = M_ERR;
	
	filebuf_free(fb);
	
	safe_strcpy(tag_track, get_corlett_title(), 256);
	safe_strcpy(tag_author, get_corlett_artist(), 256);
	safe_strcpy(tag_game, get_corlett_game(), 256);
	
	safe_strcpy(tag_system, "Playstation", 256);
	
	ao_file_open = 1;
	
	return fok;
}

void close_psf_file()
{
	if (ao_file_open == 0)
		return;
	
	if (  play_stat==M_PLAY ||
		  play_stat==M_DO_STOP || 
		  play_stat==M_DO_ERR_STOP ||
		  play_stat==M_RELOAD ||
		  play_stat==M_RELOAD_IDLE)
		psf_stop();
	
	if (play_stat==M_DO_ERR_STOP)
		play_stat=M_ERR;
	else if (play_stat != M_RELOAD && play_stat != M_RELOAD_IDLE)
		play_stat=M_STOPPED;
	
	ao_file_open = 0;
}

int load_psf2_file(char *fn)
{
	if (ao_file_open == 1)
		return;
	
	char *ctmp;
	struct filebuf *fb = filebuf_init();
	int tlen, fok;
	FILE *fp;
	
	clear_tags();
	
	u_int8_t *tbuf;
	
	if (fn != 0)
	{
		ctmp = strip_dir(fn);
		
		ao_set_lib_dir(ctmp);
	}
	
	filebuf_load(fn, fb);
	
	if (fb->len==0)
	{
		play_stat = M_ERR;
		
		return 0;
	}
	fok = psf2_start(fb->buf, fb->len);
	
	if (fok == AO_FAIL)
		play_stat = M_ERR;
	filebuf_free(fb);
	
	safe_strcpy(tag_track, get_corlett_title(), 256);
	safe_strcpy(tag_author, get_corlett_artist(), 256);
	safe_strcpy(tag_game, get_corlett_game(), 256);
	
	safe_strcpy(tag_system, "Playstation 2", 256);
	ao_file_open = 1;
	
	return fok;
}

void close_psf2_file()
{
	if (ao_file_open == 0)
		return;
	
	psf2_stop();
	
	
	ao_file_open = 0;
}
