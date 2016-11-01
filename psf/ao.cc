
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#include "corlett.h"
#include "eng_protos.h"

#include <libaudcore/index.h>
#include <libaudcore/audstrings.h>
#include <libaudcore/vfs.h>

static String dirpath;


int ao_channel_enable[24] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

int ao_chan_disp[25*2];
/* 
 * an averaging of all samples per update, for display purposes.
 * 
 * like channel_enable, set around the 24 max channels
 * psf has.  25 is master.
 */
 
 
int ao_chan_flag_disp[24];


/* ao_get_lib: called to load secondary files */
Index<char> ao_get_lib(char *libdir, char *filename)
{
	if (!libdir)
		dirpath = (String) "file://./";
	else
		dirpath = (String) libdir;
	
	printf("lib str %s\n", libdir);
	
    VFSFile file(filename_build({dirpath, filename}), "r");
    return file ? file.read_all() : Index<char>();
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
