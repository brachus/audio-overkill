
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "ao.h"
#include "psf/corlett.h"
#include "psf/eng_protos.h"

#ifdef unix
#include <sys/stat.h>
struct stat sb;

#else
#include <Windows.h>
#include <FileAPI.h>

#endif




int is_dir(char *fn)
{
	#ifdef unix
	
	return (stat(fn, &sb) == 0 && S_ISDIR(sb.st_mode));
		
	#else
	
	DWORD dwAttrib = GetFileAttributes(fn);
	
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
		
	#endif
	
	return 0;
}

int ao_channel_enable[24] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

int ao_chan_disp[25*2];
/* 
 * an averaging of all samples per update, for display purposes.
 * 
 * like channel_enable, set around the 24 max channels
 * psf has.  25 is master.
 */

int ao_chan_disp_min[25*2] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
		
		
int ao_chan_disp_max[25*2] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};

int ao_chan_disp_nchannels = 24;
 
 
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

int sid_subsong_sel = 0;

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
		free(r->buf);
		
	r->state = 0;
	r->len = 0;
	r->buf=0;
	
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
	
	if (is_dir(fn))
		return 0;
	
		
	
	fp = fopen(fn, "rb");
	
	if (!fp)
		return 0;
	
	fseek(fp, 0L, SEEK_END);
	r->len = ftell(fp);
	rewind(fp);
	
	r->buf = (uint8_t *) malloc(r->len);
	
	fread(r->buf, sizeof(uint8_t), r->len, fp);
	
	fclose(fp);	
	
	r->state = 1;
	
	return r->state;
}


char *filename_build(const char *dir, char *fn)
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
				
				r[i] = dir[j];
				
				if (dir[j] != dr)
					md=1;
				else
					md=2;
					
				j=0;
			}
			else
				r[i] = dir[j++];
		}
		else if (md==1)
		{
			r[i] = dr;
			md=2;
		}
		else if (md==2)
		{
			r[i] = fn[j++];
			
			if (j>=flen)
			{
				r[++i] = '\0';
				break;
			}
				
		}
		
	}
	
	return r;
}

char *strip_dir(char *path)
{
	int i;
	char *slash, *npath, *nptmp, *ch;
	
	if (!path)
		return 0;
		
	slash = strrchr(path, '/');
	
	if (!slash)
		return 0;
		
	npath = (char *) malloc(strlen(path) + 1);
	
	nptmp = npath;
	
	while ((path) <= slash)
		*(nptmp++) = *(path++);
	
	*nptmp = '\0';	/* null it */
	
	return npath;
	
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
		
	/*printf("loading %s...\n", tmpdir);*/
    
    
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
	
	static int sp_acc[25*2]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	int refr=44100/60, tmpl, tmpr;
	
	l=l<0?-l:l;
	r=r<0?-r:r;
	
	if (sp_cnt[ch] % (refr) == 0) /* either refresh or min-max */
	{
		ao_chan_disp[ch*2] = sp_acc[ch*2] / refr;//l;
		ao_chan_disp[(ch*2)+1] = sp_acc[ch*2+1] / refr;//r;
		
		sp_acc[ch*2] = 0;
		sp_acc[ch*2+1] = 0;
	}
	else
	{
		/*
		if (l>ao_chan_disp[ch*2])
			ao_chan_disp[ch*2] = l;
		
		if (r>ao_chan_disp[ch*2+1])
			ao_chan_disp[ch*2+1] = r;*/
		
		sp_acc[ch*2] += l;
		sp_acc[ch*2+1] += r;
			
		
	}
	
	/* update min-max.  this will be the window for displaying the values. */
	
	tmpl=l;tmpr=r;
	
	tmpl=ao_chan_disp[ch*2];
	tmpr=ao_chan_disp[ch*2+1];
	
	ao_chan_disp_min[ch*2] = (tmpl<ao_chan_disp_min[ch*2]) ? tmpl : ao_chan_disp_min[ch*2];
	ao_chan_disp_min[ch*2+1] = (tmpr<ao_chan_disp_min[ch*2+1]) ? tmpr : ao_chan_disp_min[ch*2+1];
	
	ao_chan_disp_max[ch*2] = (tmpl>ao_chan_disp_max[ch*2]) ? tmpl : ao_chan_disp_max[ch*2];
	ao_chan_disp_max[ch*2+1] = (tmpr>ao_chan_disp_max[ch*2+1]) ? tmpr : ao_chan_disp_max[ch*2+1];
	
	sp_cnt[ch]++;
	
}

void reset_chan_disp()
{
	
	int i;
	
	for (i=0;i<(24*2);i++)
		ao_chan_disp[i] = 0;
	for (i=0;i<(24*2);i++)
		ao_chan_disp_min[i] = 0;
	for (i=0;i<(24*2);i++)
		ao_chan_disp_max[i] = 0;
		
	ao_chan_disp_nchannels = 0;
 
	for (i=0;i<(24);i++)
		ao_chan_flag_disp[i]=0x888888;
		
}

void clear_tags()
{
	strcpy(tag_track, "???");
	strcpy(tag_author, "???");
	strcpy(tag_game, "???");
	strcpy(tag_system, "???");
	strcpy(tag_year, "???");
	strcpy(tag_notes, "???");
	strcpy(tag_chips, "\0");
}
