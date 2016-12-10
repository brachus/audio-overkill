
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <libgen.h>

#include <glib.h>

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

int ao_track_select = 0;
int ao_track_max = -1;

int ao_sample_rate = 44100;

int ao_set_spc_echo = 1;

char ao_lib_dir[256] = "\0";

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

char *strip_dir(const char *path)
{	
	char *r;
	int a,b,lash;
	
	if (path==0)
		return 0;
	
	r = (char *) malloc(strlen(path) + 1);
	
	if (!r)
		return 0;
	
	/* find last slash */
	
	lash=-1;
	a=0;
	while (path[a] !='\0' )
	{
		if (path[a] == '/')
			lash=a;
		a++;
	}
	
	a=lash;
	
	if (lash==-1)
	{
		free(r);
		return 0;
	}
		
	
	if (path[a] == '\0')
	{
		free(r);
		return 0;
	}
		
	a++; /* strlen path to last slash */
	
	for (b=0;b<(a+1);b++)
	{
		if (b<a)
			r[b] = path[b];
		else
			r[b] = '\0';
	}
	
	
	return r;
	
	
}

char *strip_fn(const char *path)
{	
	char *r;
	int a,b,lash;
	
		
	if (!path)
		return 0;
	
	r = (char *) malloc(strlen(path) + 1);
	
	if (!r)
		return 0;
	
	/* find last slash */
		
	lash=-1;
	a=0;
	while (path[a] != '\0' )
	{
		if (path[a] == '/')
			lash = a;
		a++;
	}
	
	
	if (lash==-1)
	{
		free(r);
		return 0;
	}
	
	a=lash;
	
	if (path[a] == '\0')
	{
		strcpy(r, path);
		return r;
	}
	
	a++; /*after slash*/
	
	b=0;
	r[b] = '\0';
	while (path[a] != '\0')
	{
		r[b++] = path[a++];
		r[b] = '\0';
	}
		
	return r;
	
}

void ao_set_lib_dir(char *s)
{
	ao_lib_dir[0] = '\0';
	
	strcpy(ao_lib_dir, s);
	
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
	
	/*printf("tmpdir: %s\n",tmpdir);*/
    
    r = filebuf_load(tmpdir, fbuf);
    
    free(tmpdir);
    
    return r;
    
    /*return file ? file.read_all() : Index<char>();*/
}

char *ao_chip_names[] = 
{
	"PSF",
	"PSF2",
	"YM2612",
	"AY8910",
	"C6280",
	"YM2149",
	"K051649",
	"YM2203",
	"YM2608",
	"NES APU",
	"NES APU",
	"OKI M6258",
	"OKI M6295",
	"C6280",
	"SEGA PCM",
	"FM 2612",
	"QSOUND",
	"SN76489",
	"SN76496",
	"YM2151",
	
	"SID",
	
	"NSF",
	"NINTENDO S-SMP",
	"KSS"
};

int ao_channel_set_chip[4];
int ao_channel_mix[4*128*2];
int ao_channel_nchannels[4];
int ao_channel_min[4*128];
int ao_channel_max[4*128];
int ao_chan_flag_disp[4*128];

int ao_channel_mix_update_cnt1[4*128];

int ao_channel_mix_update_cnt[4*128];
int ao_channel_mix_update_acc[4*128*2];

#define _AO_H_MAX_DISP_CHAN 128

void mix_chan_disp_flush()
{
	int i, cntch, curch, ch_idx, ch_left, ch;
	
	for (i=0;i<4;i++)
		if (ao_channel_set_chip[i]==-1)
		{
			cntch = i;
			break;
		}
	
	if (cntch < 1)
		return;
	
	for (curch=0;curch<cntch;curch++)
	{
		for (ch=0;ch<ao_channel_nchannels[curch];ch++)
		{
			ch_idx = (curch * _AO_H_MAX_DISP_CHAN) + ch;
			ch_left = ((curch * _AO_H_MAX_DISP_CHAN) + ch) * 2;
			
			if (ao_channel_mix_update_cnt[ch_idx] != 0)
			{
				ao_channel_mix[ch_left] =
					ao_channel_mix_update_acc[ch_left] /
						ao_channel_mix_update_cnt[ch_idx];//l;
				ao_channel_mix[ch_left + 1] =
					ao_channel_mix_update_acc[ch_left + 1] /
						ao_channel_mix_update_cnt[ch_idx];//r;
				
				ao_channel_mix_update_acc[ch_left] = 0;
				ao_channel_mix_update_acc[ch_left + 1] = 0;
			}
			
			ao_channel_mix_update_cnt[ch_idx] = 0;
		}
		
	}
}

void mix_chan_disp(int chip_id, int nchannels, int ch, short l, short r)
{
	
	int tmpl, tmpr, cmatch = -1, i, j, ch_idx, ch_left;
	
	for (i=0;i<4;i++)
	{
		if (chip_id == ao_channel_set_chip[i])
		{
			cmatch = i;
			break;
		}
		if (ao_channel_set_chip[i] == -1)
		{
			cmatch = i;
			
			ao_channel_set_chip[cmatch] = chip_id;
			ao_channel_nchannels[cmatch] = nchannels;
			
			/* refresh chip names string here */
			
			tag_chips[0] = '\0';
			for (j=cmatch;j>=0;j--)
			{
				strcat(tag_chips, ao_chip_names[ao_channel_set_chip[j]]);
				strcat(tag_chips, " ");

			}
			
			
			
			break;
		}
	}
		
	if (cmatch < 0)
		return;
		
	
	
	
	ch_idx = (cmatch * _AO_H_MAX_DISP_CHAN) + ch;
	ch_left = ((cmatch * _AO_H_MAX_DISP_CHAN) + ch) * 2;
	
	
	l=l<0?-l:l;
	r=r<0?-r:r;
	
	
	ao_channel_mix_update_acc[ch_left] += l;
	ao_channel_mix_update_acc[ch_left + 1] += r;
	
	/* update min-max.  this will be the window for displaying the values. */
	
	tmpl = ao_channel_mix[ch_left];
	tmpr = ao_channel_mix[ch_left + 1];
	
	if (tmpl < ao_channel_min[ch_idx])
		ao_channel_min[ch_idx] = tmpl;
	
	if (tmpr < ao_channel_min[ch_idx])
		ao_channel_min[ch_idx] = tmpr;
	
	ao_channel_min[ch_idx] = 0;
	
	if (ao_channel_mix_update_cnt1[ch_idx]>128 &&
		ao_channel_mix_update_cnt1[ch_idx] % 32 == 0 && 
		ao_channel_max[ch_idx] > 2)
		ao_channel_max[ch_idx]--;
	
	if (	ao_channel_max[ch_idx] > 2 &&
			tmpl < (ao_channel_max[ch_idx]) &&
			tmpr < (ao_channel_max[ch_idx]))
		ao_channel_mix_update_cnt1[ch_idx]++;
	else
		ao_channel_mix_update_cnt1[ch_idx] = 0;
		
	
	if (tmpl > ao_channel_max[ch_idx])
		ao_channel_max[ch_idx] = tmpl;
	
	if (tmpr > ao_channel_max[ch_idx])
		ao_channel_max[ch_idx] = tmpr;
	
	
	ao_channel_mix_update_cnt[ch_idx]++;
	
}

void reset_chan_disp()
{
	
	int i;
	
	for (i=0;i<(4);i++)
	{
		ao_channel_nchannels[i] = 0;
		ao_channel_set_chip[i] = -1;
	}
		
	for (i=0;i<(4*128*2);i++)
		ao_channel_mix[i] = 0;
		
	for (i=0;i<(4*128);i++)
	{
		ao_channel_mix_update_cnt[i] = 0;
		ao_channel_mix_update_cnt1[i] = 0;
		
		ao_channel_mix_update_acc[i] = 0;
		
		ao_chan_flag_disp[i] = 0x888888;
		
		ao_channel_max[i] = 0;
		ao_channel_min[i] = 0;
	}
		
		
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

void safe_strcpy(char *dst, char *src, int lim)
{
	int i = 0;
	
	if (!src)
		return;
	
	while (i<lim)
	{
		if (i == (lim-1) || src[i] == '\0')
		{
			dst[i] = '\0';
			return;
		}
		
		dst[i] = src[i];
		
		i++;
	}		
}



/* ascii only */
int strcmp_nocase (const char * a, const char * b, int len)
{
    if (! a)
        return b ? -1 : 0;
    if (! b)
        return 1;

    return len < 0 ? g_ascii_strcasecmp (a, b) : g_ascii_strncasecmp (a, b, len);
}


int ao_file_open = 0;

