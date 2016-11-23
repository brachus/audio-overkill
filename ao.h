//
// Audio Overload SDK
//
// Fake ao.h to set up the general Audio Overload style environment
//

#ifndef __AO_H
#define __AO_H

#include <stdint.h>


#define WANT_AUD_BSWAP
/*
#include <libaudcore/audio.h>
#include <libaudcore/index.h>
*/

#ifdef WANT_AUD_BSWAP



#undef bswap16
#undef bswap32
#undef bswap64

/* GCC will optimize these to appropriate bswap instructions 
extern uint16_t bswap16 (uint16_t x)
    { return ((x & 0xff00) >> 8) | ((x & 0x00ff) << 8); }

extern uint32_t bswap32 (uint32_t x)
{
    return ((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >> 8) |
           ((x & 0x0000ff00) << 8) | ((x & 0x000000ff) << 24);
}

extern uint64_t bswap64 (uint64_t x)
{
    return ((x & 0xff00000000000000) >> 56) | ((x & 0x00ff000000000000) >> 40) |
           ((x & 0x0000ff0000000000) >> 24) | ((x & 0x000000ff00000000) >> 8) |
           ((x & 0x00000000ff000000) << 8) | ((x & 0x0000000000ff0000) << 24) |
           ((x & 0x000000000000ff00) << 40) | ((x & 0x00000000000000ff) << 56);
}*/

#endif // WANT_AUD_BSWAP

#if 0 // BIGENDIAN

#define FMT_S16_NE FMT_S16_BE
#define FMT_U16_NE FMT_U16_BE
#define FMT_S24_NE FMT_S24_BE
#define FMT_U24_NE FMT_U24_BE
#define FMT_S32_NE FMT_S32_BE
#define FMT_U32_NE FMT_U32_BE

#ifdef WANT_AUD_BSWAP
#define FROM_BE16(x) (x)
#define FROM_BE32(x) (x)
#define FROM_BE64(x) (x)
#define FROM_LE16(x) (bswap16 (x))
#define FROM_LE32(x) (bswap32 (x))
#define FROM_LE64(x) (bswap64 (x))
#define TO_BE16(x) (x)
#define TO_BE32(x) (x)
#define TO_BE64(x) (x)
#define TO_LE16(x) (bswap16 (x))
#define TO_LE32(x) (bswap32 (x))
#define TO_LE64(x) (bswap64 (x))
#endif

#else  // ! BIGENDIAN

#define FMT_S16_NE FMT_S16_LE
#define FMT_U16_NE FMT_U16_LE
#define FMT_S24_NE FMT_S24_LE
#define FMT_U24_NE FMT_U24_LE
#define FMT_S32_NE FMT_S32_LE
#define FMT_U32_NE FMT_U32_LE

#ifdef WANT_AUD_BSWAP
#define FROM_BE16(x) (bswap16 (x))
#define FROM_BE32(x) (bswap32 (x))
#define FROM_BE64(x) (bswap64 (x))
#define FROM_LE16(x) (x)
#define FROM_LE32(x) (x)
#define FROM_LE64(x) (x)
#define TO_BE16(x) (bswap16 (x))
#define TO_BE32(x) (bswap32 (x))
#define TO_BE64(x) (bswap64 (x))
#define TO_LE16(x) (x)
#define TO_LE32(x) (x)
#define TO_LE64(x) (x)
#endif

#endif


/*#define UINT32 uint32_t
#define UINT16 uint16_t
#define UINT8 uint8_t
#define INT32 int32_t
#define INT16 int16_t
#define INT8 int8_t*/
#define u_int32_t uint32_t
#define u_int16_t uint16_t
#define u_int8_t uint8_t


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

int is_dir(char *fn);

struct filebuf *filebuf_init();
int filebuf_free(struct filebuf *r);
int filebuf_load(char *fn, struct filebuf *r);
char * filename_build(const char *dir, char *fn);

char *strip_dir(char *path);

int ao_get_lib(struct filebuf *fbuf, char *libdir, char *filename);

extern int ao_channel_enable[24];
extern int ao_chan_disp[25*2];
extern int ao_chan_flag_disp[24];

extern int ao_chan_disp_min[25*2];
extern int ao_chan_disp_max[25*2];

extern int ao_chan_disp_nchannels;

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

void reset_chan_disp();

void clear_tags();

extern char tag_track[256];
extern char tag_author[256];
extern char tag_game[256];
extern char tag_system[256];
extern char tag_year[256];
extern char tag_notes[256];
extern char tag_chips[1024];

extern int play_stat;

/* get rid of sid_subsong_sel */
extern int sid_subsong_sel;

enum
{
	M_PLAY,
	M_ERR,
	M_PAUSE,
	M_DO_STOP,
	M_DO_ERR_STOP,
	M_STOPPED,
	M_LOAD,
	M_RELOAD,
	M_RELOAD_IDLE
};


#endif // AO_H
