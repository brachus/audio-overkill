
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "ao.h"

#include "corlett.h"
#include "eng_protos.h"

/*#include <libaudcore/index.h>
#include <libaudcore/audstrings.h>
#include <libaudcore/vfs.h>*/

//static String dirpath;


int ao_channel_enable[24] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

int ao_chan_disp[25*2];
/* 
 * an averaging of all samples per update, for display purposes.
 * 
 * like channel_enable, set around the 24 max channels
 * psf has.  25 is master.
 */
 
 
int ao_chan_flag_disp[24];

int ao_sample_idx[64];
int ao_sample_do[64] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	};
int ao_sample_idx_cur = 0;
int ao_sample_limit[2] = {0, 2};

int ao_set_len=~0;

void ao_add_sample(int sndtick, int sample)
{
	static int idx_len = 0;
	int i;
	
	if (sndtick % 128 == 0 && ao_sample_limit[0] == 1)
	{
		if (ao_sample_limit[0] == 0)
			return;
		
		for (i=0;i<ao_sample_idx_cur;i++)
			if (ao_sample_idx[i] == sample)
				return;
		
		if (ao_sample_idx_cur<63)
		{
			ao_sample_idx_cur++;
			ao_sample_idx[ao_sample_idx_cur-1] = sample;
			printf("added %d\n",sample);
		}
	}
	
}

int ao_sample_limit_ok(int sample)
{
	int i;
	
	if (ao_sample_limit[0]==0)
		return 1;
	
	/*printf("%d %d\n",sample, ao_sample_idx[ao_sample_limit[1]]);*/
	
	for(i=0;i<ao_sample_idx_cur;i++)
		if (sample == ao_sample_idx[i] && ao_sample_do[i])
			return 1;
	
	return 0;
}

void ao_sample_idx_clear()
{
	ao_sample_idx_cur=0;
}

struct filebuf *filebuf_init()
{
	struct filebuf *r = (struct filebuf *) malloc(sizeof(struct filebuf));
	
	r->len=0;
	r->state=0;
	r->buf=0;
	
	return r;
}

int filebuf_free(struct filebuf *r)
{
	if (r->state == 1)
	{
		free(r->buf);
		r->state = 0;
		r->len = 0;
		r->buf=0;
	}
	
	return r->state;
}

int filebuf_load(char *fn, struct filebuf *r)
{
	FILE *fp;
	char ch;
	long i;
	
	filebuf_free(r);
	
	if (!fn)
		return 0;
	
	fp = fopen(fn, "r");
	
	if (!fp)
		return 0;
	
	fseek(fp, 0L, SEEK_END);
	r->len = ftell(fp);
	rewind(fp);
	
	r->buf = (uint8_t *) malloc(r->len);
	
	fread(r->buf, sizeof(uint8_t), r->len, fp);
	
	fclose(fp);	
	
	return r->state;
}


char *filename_build(char *dir, char *fn)
{
	int i, j, dlen, flen, len, md;
	char dr='/';
	
	#if defined(_WIN32) || defined(WIN32)
		dr = '\\';
	#endif
	
	dlen = strlen(dir);
	flen = strlen(fn);
	
	if (dlen==0 || flen==0)
		return 0;
	
	len = dlen + flen + 2;
	
	char *r = (char *) malloc(len);
	
	md=0;
	j=0;
	
	for (i=0;i<len;i++)
	{
		if (md==0)
		{
			if (j == dlen-1)
			{
				
				if (dir[j] != dr)
					r[i] = dir[j];
				
				md=1;
				j=0;
			}
			else
			{
				r[i] = dir[j];
				j++;
			}
				
		}
		if (md==1)
		{
			r[i] = dr;
			md=2;
		}
		else if (md==2)
		{
			r[i] = fn[j];
			
			j++;
			
			if (j>=flen)
			{
				r[i+1] = '\0';
				break;
			}
				
		}
		
	}
	
	return r;
	
	
}


/* ao_get_lib: called to load secondary files */
int ao_get_lib(struct filebuf *fbuf, char *libdir, char *filename)
{
	char *tmpdir;
	int r;
	
	
    /*VFSFile file(filename_build({dirpath, filename}), "r");*/
    
    if (!libdir)
		tmpdir = filename_build("./", filename);
	else
		tmpdir = filename_build(libdir, filename);
		
	printf("loading %s...\n", tmpdir);
    
    
    //r = filebuf_load((char *) filename_build({dirpath, filename}), fbuf);
    r = filebuf_load(tmpdir, fbuf);
    
    free(tmpdir);
    
    return r;
    
    /*return file ? file.read_all() : Index<char>();*/
}



void set_channel_enable(int *set)
{
	int i;
	
	for (i=0;i<24;i++)
		ao_channel_enable[i] = set[i];
}

void set_chan_disp(int ch, short l, short r)
{
	//ao_chan_disp[ch*2] = l;
	//ao_chan_disp[(ch*2)+1] = r;
	return;
	
}

void mix_chan_disp(int ch, short l, short r)
{
	static int sp_cnt[25]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	l=l<0?-l:l;
	r=r<0?-r:r;
	
	if (sp_cnt[ch] % (44100/60) == 0)
	{
		ao_chan_disp[ch*2] = l;
		ao_chan_disp[(ch*2)+1] = r;
	}
	else
	{
		if (l>ao_chan_disp[ch*2])
			ao_chan_disp[ch*2] = l;
		
		if (r>ao_chan_disp[ch*2+1])
			ao_chan_disp[ch*2+1] = r;
		
		/*ao_chan_disp[ch*2] += l;
		ao_chan_disp[ch*2] /= 2;
		ao_chan_disp[(ch*2)+1] += r;
		ao_chan_disp[(ch*2)+1] /= 2;*/
	}
	
	sp_cnt[ch]++;
	
}
