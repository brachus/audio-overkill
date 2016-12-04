

#include "tinysid.h"

/* this has to be 882*2 or else. */
#define SID_SAMPLERATE (882*2)

static word init_addr, play_addr;
static byte actual_subsong, max_subsong, play_speed;
static char song_name[32], song_author[32], song_copyright[32], subsong_disp[32];

static word *wave_buf;


void sid_fill_tags();
int sid_execute ( void (*update)(const void *, int ));
int sid_open ( char * fn);
void sid_close ( void );
