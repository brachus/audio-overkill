
#include "dsf_plugin.h"


int16_t *dsf_output = 0;


int load_dsf_file(char *fn)
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
	
	fok = dsf_start((uint8 *) fb->buf, (uint32) fb->len);
	
	if (fok == AO_FAIL)
		play_stat = M_ERR;
	
	filebuf_free(fb);
	
	strcpy(tag_system, "Sega Dreamcast");
	
	dsf_output = malloc( sizeof (int16_t) * 2 * DSF_SAMPLERATE);
	
	
	ao_file_open = 1;
	
	return fok;
}

void close_dsf_file()
{
	if (ao_file_open == 0)
		return;
	
	if (dsf_output != 0)
	{
		free(dsf_output);
		dsf_output = 0;
	}
		
	
	if (  play_stat==M_PLAY ||
		  play_stat==M_DO_STOP || 
		  play_stat==M_DO_ERR_STOP ||
		  play_stat==M_RELOAD ||
		  play_stat==M_RELOAD_IDLE)
		dsf_stop();
	
	if (play_stat==M_DO_ERR_STOP)
		play_stat=M_ERR;
	else if (play_stat != M_RELOAD && play_stat != M_RELOAD_IDLE)
		play_stat=M_STOPPED;
	
	ao_file_open = 0;
}

int dsf_execute ( void (*update)(const void *, int ))
{
	int i;
	int16_t l, r;
	
	
	for (i=0;i<DSF_SAMPLERATE;i++)
	{
		dsf_sample(&l,&r);
		
		dsf_output[i * 2] = l;
		dsf_output[(i * 2) + 1] = r;
	}
	
	update((u_int8_t *) dsf_output, sizeof(int16_t) * 2 * DSF_SAMPLERATE);
	
	return 1;
	
}

