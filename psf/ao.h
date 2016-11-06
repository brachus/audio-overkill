//
// Audio Overload SDK
//
// Fake ao.h to set up the general Audio Overload style environment
//

#ifndef __AO_H
#define __AO_H

#include <stdint.h>

#define WANT_AUD_BSWAP
#include <libaudcore/audio.h>
#include <libaudcore/index.h>

#define AO_SUCCESS					1
#define AO_FAIL						0
#define AO_FAIL_DECOMPRESSION		-1

#define MAX_DISP_INFO_LENGTH		256
#define AUDIO_RATE					(44100)

enum
{
	COMMAND_NONE = 0,
	COMMAND_PREV,
	COMMAND_NEXT,
	COMMAND_RESTART,
	COMMAND_HAS_PREV,
	COMMAND_HAS_NEXT,
	COMMAND_GET_MIN,
	COMMAND_GET_MAX,
	COMMAND_JUMP
};

struct filebuf
{
	int state;
	uint8_t *buf;
	long len;
};

struct filebuf *filebuf_init();
int filebuf_free(struct filebuf *r);
int filebuf_load(char *fn, struct filebuf *r);

char * filename_build(char *dir, char *fn);

int ao_get_lib(struct filebuf *fbuf, char *libdir, char *filename);

extern int ao_channel_enable[24];

extern int ao_chan_disp[25*2];

extern int ao_chan_flag_disp[24];

extern int ao_sample_idx[64];
extern int ao_sample_do[64];
extern int ao_sample_cur;
extern int ao_sample_limit[2];

extern int ao_set_len;

void set_channel_enable(int *set);

void set_chan_disp(int ch, short l, short r);

void mix_chan_disp(int ch, short l, short r);

void ao_add_sample(int sndtick, int sample);

int ao_sample_limit_ok(int sample);

void ao_sample_idx_clear();


#endif // AO_H
