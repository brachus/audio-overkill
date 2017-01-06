//
// Audio Overload SDK
//
//

#ifndef __AO_H
#define __AO_H

#include <stdint.h>


#define WANT_AUD_BSWAP


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



typedef unsigned char ao_bool;

#ifdef __GNUC__
#include <stddef.h>	// get NULL
#include <stdbool.h>

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#endif

#ifdef _MSC_VER
#include <stddef.h>	// get NULL

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define true (1)
#define false (0)

#define strcasecmp _strcmpi

#endif

typedef unsigned char		uint8;
typedef unsigned char		UINT8;
typedef signed char			int8;
typedef signed char			INT8;
typedef unsigned short		uint16;
typedef unsigned short		UINT16;
typedef signed short		int16;
typedef signed short		INT16;
typedef signed int			int32;
typedef unsigned int		uint32;
#ifdef LONG_IS_64BIT
typedef signed long             int64;
typedef unsigned long           uint64;
#else
typedef signed long long	int64;
typedef unsigned long long	uint64;
#endif

#ifdef WIN32
/*#include "win32_utf8/src/entry.h"*/

#ifndef _BASETSD_H
typedef signed int			INT32;
typedef unsigned int		UINT32;
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
#endif
#else
typedef signed int			INT32;
typedef unsigned int		UINT32;
#ifdef LONG_IS_64BIT
typedef signed long         INT64;
typedef unsigned long       UINT64;
#else
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
#endif
#endif

#define __LITTLE_ENDIAN__ 1
/* fix this mess. */

#ifdef __BIG_ENDIAN__
	#undef LSB_FIRST
#endif

#ifdef __LITTLE_ENDIAN__
	#define LSB_FIRST	1
#endif

#ifndef INLINE
#if defined(_MSC_VER)
#define INLINE static __forceinline
#elif defined(__GNUC__)
#define INLINE static __inline__
#elif defined(_MWERKS_)
#define INLINE static inline
#elif defined(__powerc)
#define INLINE static inline
#else
#define INLINE
#endif
#endif

INLINE uint16 SWAP16(uint16 x)
{
	return (
		((x & 0xFF00) >> 8) |
		((x & 0x00FF) << 8)
	);
}

INLINE uint32 SWAP24(uint32 x)
{
	return (
		((x & 0xFF0000) >> 16) |
		((x & 0x00FF00) << 0) |
		((x & 0x0000FF) << 16)
	);
}

INLINE uint32 SWAP32(uint32 x)
{
	return (
		((x & 0xFF000000) >> 24) |
		((x & 0x00FF0000) >> 8) |
		((x & 0x0000FF00) << 8) |
		((x & 0x000000FF) << 24)
	);
}

#if LSB_FIRST
#define BE16(x) (SWAP16(x))
#define BE24(x) (SWAP24(x))
#define BE32(x) (SWAP32(x))
#define LE16(x) (x)
#define LE24(x) (x)
#define LE32(x) (x)

#ifndef __ENDIAN__ /* Mac OS X Endian header has this function in it */
#define Endian32_Swap(x) (SWAP32(res))
#endif

#else

#define BE16(x) (x)
#define BE24(x) (x)
#define BE32(x) (x)
#define LE16(x) (SWAP16(x))
#define LE24(x) (SWAP24(x))
#define LE32(x) (SWAP32(x))

#endif


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
	long seek;
};

int is_dir(const char *fn);

#define _AO_FBUF_CUR	(0)
#define _AO_FBUF_SET	(1)
#define _AO_FBUF_END	(2)

struct filebuf *filebuf_init();
int filebuf_free(struct filebuf *r);
int filebuf_load(const char *fn, struct filebuf *r);
long filebuf_fread(void *buffer, long size, long nmemb, struct filebuf *f);
long filebuf_fseek(struct filebuf * file, long offset, int whence);


char * filename_build(const char *dir, const char *fn);

char *strip_dir(const char *path);
char *strip_fn(const char *path);

int ao_get_lib_newer(const char *filename, uint8 **buffer, uint64 *length);
int ao_get_lib(struct filebuf *fbuf, char *libdir, char *filename);

extern char *ao_chip_names[];


enum
{
	_AO_H_PSF,
	
	_AO_H_PSF2,
	
	_AO_H_YM2612, /* must stay 2 */
	
	_AO_H_AY8910,
	_AO_H_C6280,
	_AO_H_EMU2149,
	_AO_H_K051649,
	_AO_H_YM2203,
	_AO_H_YM2608,
	_AO_H_NES_APU,
	_AO_H_NP_NES,
	_AO_H_OKIM6258,
	_AO_H_OKIM6295,
	_AO_H_OOTAKE_PSG,
	_AO_H_SEGA_PCM,
	_AO_H_FM_2612,
	_AO_H_QSOUND,
	_AO_H_SN76489,
	_AO_H_SN76496,
	_AO_H_YM2151,
	_AO_H_MULTIPCM,
	
	
	
	/* fill out chips supported in vgm format here */
	
	
	_AO_H_SID,
	
	
	_AO_H_GME_NSF,
	_AO_H_GME_SPC,
	_AO_H_GME_KSS,
	_AO_H_GME_GB,
	/* fill out chips supported in blargg's game music emulator here */
	
	
	_AO_H_GSF,
	_AO_H_USF,
	
	_AO_H_DSF,
	_AO_H_SSF,
	
	_AO_H_DUH
};

extern int ao_channel_set_chip[4];
extern int ao_channel_mix[4*128*2];
extern int ao_channel_nchannels[4];
extern int ao_channel_min[4*128];
extern int ao_channel_max[4*128];
extern int ao_chan_flag_disp[4*128];
extern int ao_channel_tmp_cur;
extern int ao_channel_tmp_max;

extern int ao_channel_mix_update_cnt1[4*128];

extern int ao_channel_mix_update_cnt[4*128];
extern int ao_channel_mix_update_acc[4*128*2];
extern int ao_channel_mix_update_prev[4*128*2];


extern int ao_sample_idx[64];
extern int ao_sample_do[64];
extern int ao_sample_cur;
extern int ao_sample_limit[2];

extern int ao_set_len;

void mix_chan_disp_flush();

void mix_chan_disp(int chip_id, int nchannels,int ch, int l, int r);

int mix_chan_find_avail_chip(int chip_id, int nchannels);

void mix_chan_flag(int chip_id, int nchannels, int ch, int samp);

void ao_add_sample(int sndtick, int sample);

int ao_sample_limit_ok(int sample);

void ao_sample_idx_clear();

void reset_chan_disp();

void clear_tags();

/* DESTROY THIS!!! USE strncpy!!!  (strncpy doen't add null btw. )*/
void safe_strcpy(char *dst, char *src, int lim);

int strcmp_nocase (const char * a, const char * b, int len);

extern char tag_track[256];
extern char tag_author[256];
extern char tag_game[256];
extern char tag_system[256];
extern char tag_year[256];
extern char tag_notes[256];
extern char tag_chips[1024];

extern int play_stat;

extern int ao_file_open;

extern int ao_track_select;
extern int ao_track_max;

extern int ao_sample_rate;

extern int ao_set_spc_echo;
extern int ao_play_next_on_stop;

extern char ao_lib_dir[256];

void ao_set_lib_dir(char *s);

extern int f_enable_scope_disp;
extern int f_enable_channel_disp;



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
	M_RELOAD_IDLE,
	M_NOFILE
};


#endif // AO_H
