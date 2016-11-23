

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>



#include <zlib.h>

#include "ao.h"

#include "conf.h"

#include "psf/plugin.h"

#include "vgm/plugin.h"

#include "sid/plugin.h"



#include "filelist.h"

#define DEBUG_MAIN (0)


#define TEXT_SCROLL_GAP 32

#define CONFIG_FNAME "ao.conf"

#define WIN_TITLE "Audio Overkill"

#define MAXCHAN 24


static int c_always_ontop = 0;

static int c_screen_w = 320;
static int c_screen_h = 240;
#define SCREENWIDTH c_screen_w
#define SCREENHEIGHT c_screen_h
#define SCOPEWIDTH c_screen_w

static int c_fps = 60;
#define FPS c_fps

static int col_text[3] = {50,50,50};
static int col_text_bg[3] = {230,230,250};
static int col_bg[3] = {255, 255, 255};
static int col_scope[3] = {0, 0, 0};
static int col_scope_low[3] = {255, 0, 0};
static int col_scope_high[3] = {0, 0, 255};
#define TEXT_COLOR col_text[0],col_text[1],col_text[2]
#define TEXT_BG_COLOR col_text_bg[0],col_text_bg[1],col_text_bg[2]
#define BG_COLOR col_bg[0],col_bg[1],col_bg[2]
#define SCOPE_COLOR col_scope[0],col_scope[1],col_scope[2]
#define SCOPE_LOW_COLOR col_scope_low[0],col_scope_low[1],col_scope_low[2]

static int c_scope_do_grad = 1;

static char c_ttf_font[256] = "ttf/gohufont-11.ttf";
#define TTF_FONT c_ttf_font


static int c_buf_size = (44100/30);
#define ABUFSIZ c_buf_size

static int f_borderless = 0;

static int16_t * scope;
static int16_t scope_buf[1024];
static int scope_bufsiz = -1;
static int scope_clear = 1;
static int scope_dump_max = 1024;

static float scope_view = 0.5;

static int mono = 0;

static int channel_enable[MAXCHAN] =
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};


static int u_size = 3000;
static Uint8  *u_buf;
static int u_buf_fill_start = 0;
	/* where sdl begins to fill audio stream from */
static int u_buf_fill = 0;
	/* size in bytes filled from fill_start and on.*/


#define STEREO_FILL_SIZE (c_buf_size*2)


static SDL_PixelFormat *pw_fmt;
static int pw_bpp, pw_pitch;
static uint32_t pw_ucol;
static uint8_t *pw_pixel;


static int f_render_font = 1;


static int k_shift=0;

char tag_track[256];
char tag_author[256];
char tag_game[256];
char tag_system[256];
char tag_year[256];
char tag_notes[256];
char tag_chips[1024];

enum
{
	D_TRACK,
	D_AUTHOR,
	D_GAME,
	D_SYSTEM,
	D_YEAR,
	D_NOTES,
	D_CHIPS
	
};

int info_disp[7] = {1,1,1,1,0,0,1};

enum
{
	F_UNKNOWN,
	F_PSF,
	F_PSF2,
	F_USF,
	F_VGM,
	F_SID
};

int play_stat = M_STOPPED;

static void (*file_close)(void);
static int (*file_execute)(void (*update )(const void *, int));
static int (*file_open)(char *);

int limint(int in, int from, int to);
char *str_prepend(char *s, char *pre);
char *get_fn_only(char *path);
int  pw_init(SDL_Surface *in);
void pw_set_rgb(int r,int g,int b);
void pw_fill(SDL_Surface *in);
void pw_set(SDL_Surface *in, int x, int y);
void load_conf(char * fn);
void dump_scope_buf();
int cheap_is_dir(char *path);
void reset_u_buf();
void update(const Uint8 * buf, int size);
void fill_audio(void *udata, Uint8 *stream, int len);
void safe_strcpy(char *dst, char *src, int lim);
void free_scope();
void init_scope();
void clear_scope();
void update_scope(SDL_Surface *in);
void update_ao_chdisp(SDL_Surface *in);
void update_ao_ch_flag_disp(SDL_Surface *in, int md);
void render_text( SDL_Surface* dst, char *str,
	SDL_Color *col_a, SDL_Color *col_b,
	TTF_Font *font, int x, int y,
	int bold, int maxwidth, int tick);
void sdl_set_col(SDL_Color *c, int r,int  g,int  b,int  a);
void u_buf_init();
int get_file_type(char * fn);
void print_f_type(int type);
void experimental_sample_filter(int key);
int recon_file(char* fn);
void listdir_flist(struct flist_base *b, const char *name, int level);

	
int main(int argc, char *argv[])
{
	int run, i, fcnt, fcur,
		key, tickdelay,
		tick, rtmpy;
	SDL_Event e;
	char tmpstr[256];
	char chtrack, *ctmp;
	struct flist_base *flist;	
	
	load_conf(CONFIG_FNAME);
	
	SDL_AudioSpec wanted;
	SDL_Window *win;
	SDL_Surface *screen;
	SDL_Color tcol_a;
	SDL_Color tcol_b;
	SDL_Surface* tsurf;
	SDL_Rect tr;
	
	FILE *fp;
	
	sdl_set_col(&tcol_a, TEXT_COLOR, 0);
	sdl_set_col(&tcol_b, TEXT_BG_COLOR, 0);
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	u_buf_init();
	
	init_scope();
	clear_scope();
	
	TTF_Init();
	
    TTF_Font *font = TTF_OpenFont(TTF_FONT, 11);
    
    if (!font)
    {
		fprintf(stderr,
			"err: could't find \"%s\".\n no text will be rendered.\n",
			TTF_FONT);
		f_render_font=0;
	}
	
	flist = flist_init();
		
	for (i=1;i<argc;i++)
	{
		if (is_dir(argv[i]))
			listdir_flist(flist, argv[i], 0);
			
		else if (recon_file(argv[i])==1)
			add_flist_item(flist, argv[i]);
			
	}
	
	fcnt = flist->len;
	
	if (fcnt == 0)
	{
		printf("no files.\n");
		return 0;
		
	}
	fcur = 0;
	
	sid_subsong_sel=0; /* this needs to go. */
	
	win = SDL_CreateWindow(WIN_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREENWIDTH, SCREENHEIGHT,
		(SDL_WINDOW_BORDERLESS * f_borderless)
		);
		
	screen = SDL_GetWindowSurface(win);
	
	set_channel_enable(channel_enable);
	
	clear_tags();
	
	
    /* set audio format */
    wanted.freq = 44100;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;
    wanted.samples = ABUFSIZ;
    wanted.callback = fill_audio;
    wanted.userdata = 0;
    

    /* open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 )
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return 0;
    }
    
    
    SDL_PauseAudio(1);
    
    play_stat = M_LOAD;
    
    
    chtrack=0;

	tick=0;
	
	run=1;
	
	tickdelay=20;
	
	while (run)
	{
		while ( SDL_PollEvent( &e ) )
		{
			switch ( e.type )
			{
			case SDL_QUIT:
				run = 0;
				break;
			case SDL_KEYDOWN:
				
				k_shift = e.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT);
				
				switch ( e.key.keysym.sym )
				{
				case SDLK_ESCAPE:
					run = 0;
					break;
					
				case SDLK_SPACE:
					if (play_stat == M_STOPPED || play_stat == M_ERR)
						play_stat = M_LOAD;
					else if (play_stat == M_PAUSE)
						play_stat = M_PLAY;
					else if (play_stat == M_PLAY)
						play_stat = M_PAUSE;
				
					break;
				
				case SDLK_m:
					mono = (mono == 1) ? 0 : 1;
					break;
				
				case SDLK_LEFT:
				case SDLK_RIGHT:
					if (e.key.keysym.sym==SDLK_LEFT && fcur>0)
						fcur--;
					else if (fcur<(fcnt-1)) 
						fcur++;
					
					if (play_stat == M_PLAY)
						play_stat = M_RELOAD;
					else if (play_stat == M_PAUSE)
						play_stat = M_RELOAD_IDLE;
					
					if (play_stat == M_STOPPED || play_stat == M_ERR)
						play_stat = M_LOAD;
					
					break;
				
				case SDLK_0:case SDLK_1:case SDLK_2:
				case SDLK_3:case SDLK_4:case SDLK_5:
				case SDLK_6:case SDLK_7:case SDLK_8:
				case SDLK_9:case SDLK_q:case SDLK_w:
				case SDLK_e:case SDLK_r:case SDLK_t:
				case SDLK_y:case SDLK_u:case SDLK_i:
				case SDLK_o:case SDLK_p:
					experimental_sample_filter( e.key.keysym.sym );
					break;
				
				/* get rid of sid_subsong_sel */
				case SDLK_a:
				case SDLK_d:	
					if (e.key.keysym.sym==SDLK_d)
						sid_subsong_sel++;
					else
						sid_subsong_sel--;
					
					if (play_stat == M_PLAY)
						play_stat = M_RELOAD;
					else if (play_stat == M_PAUSE)
						play_stat = M_RELOAD_IDLE;
					
					if (play_stat == M_STOPPED || play_stat == M_ERR)
						play_stat = M_LOAD;
					break;
					
				case SDLK_c:
					
					load_conf(CONFIG_FNAME);
					
					/* refresh a couple things... */
					sdl_set_col(&tcol_a, TEXT_COLOR, 0);
					sdl_set_col(&tcol_b, TEXT_BG_COLOR, 0);
					free_scope();
					init_scope();
					
					SDL_SetWindowSize(win, c_screen_w, c_screen_h);
					
					/* should screen be freed ? */
					screen = SDL_GetWindowSurface(win);
					break;
				
				default:break;
				}
				break;
			default:break;
			}
		}
		
		
		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, BG_COLOR));
		
		
		if (play_stat == M_PLAY)
			dump_scope_buf();
			
		update_scope(screen);
		
		update_ao_chdisp(screen);
		update_ao_ch_flag_disp(screen, 2);
		
		rtmpy = 8;

		for (i=0;i<7;i++)
		{
			if (info_disp[i])
			{
				switch (i)
				{
				case D_TRACK:
					ctmp = (play_stat == M_ERR) ?
						get_flist_idx(flist,fcur) :
						tag_track;
					break;
				case D_AUTHOR:ctmp = tag_author;break;
				case D_GAME:ctmp = tag_game;break;
				case D_SYSTEM:ctmp = tag_system;break;
				case D_YEAR:ctmp = tag_year;break;
				case D_NOTES:ctmp = tag_notes;break;
				case D_CHIPS:ctmp = tag_chips;break;
				}
				
				render_text(
					screen,
					ctmp,
					&tcol_a,
					&tcol_b,
					font,
					10,rtmpy,
					1,SCREENWIDTH/2,tick);
					
				rtmpy += 12;
			}
		}
		
		
		
		switch (play_stat)
		{
		case M_PAUSE:
			render_text(
				screen, "- paused -", &tcol_a, &tcol_b,
				font, 10,SCREENHEIGHT - 16,1 ,SCREENWIDTH/2 ,tick);
			break;
		case M_STOPPED:
			render_text(
				screen, "- stopped -", &tcol_a, &tcol_b,
				font, 10,SCREENHEIGHT - 16,1 ,SCREENWIDTH/2 ,tick);
			break;
		case M_ERR:
			render_text(
				screen, "- error -", &tcol_a, &tcol_b,
				font, 10,SCREENHEIGHT - 16,1 ,SCREENWIDTH/2 ,tick);
			break;
		default:break;
		}
		
		
		if (mono)
			render_text(
				screen, "mono", &tcol_a, &tcol_b,
				font, SCREENWIDTH-64, 8,  1,SCREENWIDTH/2,tick);
		else
			render_text(
				screen, "stereo", &tcol_a, &tcol_b,
				font, SCREENWIDTH-64, 8,  1,SCREENWIDTH/2,tick);
				
		
		switch (play_stat)
		{
		case M_DO_STOP:
		case M_DO_ERR_STOP:
			clear_tags();
			SDL_PauseAudio(1);
			close_psf_file();
			break;
		
		case M_RELOAD_IDLE:
		case M_RELOAD:
			clear_tags();
			
			SDL_PauseAudio(1);
			
			reset_u_buf();
		
			file_close(); /* segues into M_LOAD on purpose. */
			
		case M_LOAD:
		
			SDL_PauseAudio(1);
			
			reset_u_buf();
			
			reset_chan_disp();
			
			if (get_file_type(get_flist_idx(flist,fcur)) == F_PSF)
			{
				file_close = &close_psf_file;
				file_execute = &psf_execute;
				file_open = &load_psf_file;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_VGM)
			{
				file_close = &vgm_close;
				file_execute = &vgm_execute;
				file_open = &vgm_open;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_SID)
			{
				file_close = &sid_close;
				file_execute = &sid_execute;
				file_open = &sid_open;
			}
			else
			{
				play_stat = M_ERR;
				break;
			}
			
			
			
			file_open(get_flist_idx(flist,fcur));
			
			
			clear_scope();
			
			update_scope(screen);
			
			tickdelay=20;
			tick=0;
			
			if (play_stat == M_RELOAD_IDLE)
				play_stat = M_PAUSE;
			else if (play_stat != M_ERR)
				play_stat = M_PLAY;
			
			
			break;
			
		case M_PLAY:
			SDL_PauseAudio(0);
			break;
			
		case M_PAUSE:
			SDL_PauseAudio(1);
			break;
		}
		
		sprintf(tmpstr,"(%d/%d)",fcur+1,fcnt);
		
		render_text(
			screen,
			tmpstr,
			&tcol_a,
			&tcol_b,
			font,
			10, rtmpy,
			1,SCREENWIDTH/2,tick);
		
		
		
		
		set_channel_enable(channel_enable);
		
		SDL_UpdateWindowSurface(win);
		
		
        SDL_Delay(1000/FPS);
        
        if (tickdelay < 0)
			tick++;
        else
			tickdelay--;
	
	}
	
	SDL_CloseAudio();
	close_psf_file();
	free_scope();
	
}

int pw_init(SDL_Surface *in)
{
	
	pw_fmt=in->format;
	pw_bpp = pw_fmt->BytesPerPixel;
	pw_pitch = in->pitch;
	
	return (pw_bpp==4);
		
}

void pw_set_rgb(int r,int g,int b)
{
	
	pw_ucol = SDL_MapRGB(pw_fmt, r,g,b);
}

void pw_fill(SDL_Surface *in)
{
	SDL_FillRect(in, 0, pw_ucol);
}

void pw_set(SDL_Surface *in, int x, int y)
{
	pw_pixel = (uint8_t*) in->pixels;
	pw_pixel += (y * pw_pitch) + (x * pw_bpp);
	*((uint32_t*)pw_pixel) = pw_ucol;
}

int limint(int in, int from, int to)
{
	if (in < from)
		in = from;
	if (in > to)
		in = to;
	return in;
}


void load_conf(char * fn)
{
	struct cfg_entry *o, *tmp;
	
	o = read_conf(fn);
	
	if (!o)
		return;
	
	tmp = o;
	
	#if DEBUG_MAIN
	print_cfg_entries(o);
	#endif
	
	#ifndef CONF_IS_BOOL
	 #define CONF_IS_BOOL (tmp->type==E_BOOL || tmp->type==E_INT)
	 #define ENTRY_NAME(x) (strcmp(x,tmp->name) == 0)
	 #define CONF_INT (tmp->dat[0].i)
	 #define CONF_FLOAT (tmp->dat[0].f)
	#endif
	
	while (tmp)
	{
		if (strcmp("general",tmp->section)==0)
		{
			
			if (ENTRY_NAME("fps")  && tmp->type==E_INT)
			{
				if (tmp->dat[0].i > 0)
					c_fps = limint(tmp->dat[0].i, 1, 128);
			}
			else if (ENTRY_NAME("ttf_font") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
				
			else if (ENTRY_NAME("text_color")  && tmp->type==E_RGB )
			{
				col_text[0] = limint(tmp->dat[0].i, 0, 255);
				col_text[1] = limint(tmp->dat[1].i, 0, 255);
				col_text[2] = limint(tmp->dat[2].i, 0, 255);
			}
			
			else if (ENTRY_NAME("text_bg_color") && tmp->type==E_RGB )
			{
				col_text_bg[0] = limint(tmp->dat[0].i, 0, 255);
				col_text_bg[1] = limint(tmp->dat[1].i, 0, 255);
				col_text_bg[2] = limint(tmp->dat[2].i, 0, 255);
			}
			
			else if (ENTRY_NAME("bg_color") && tmp->type==E_RGB )
			{
				col_bg[0] = limint(tmp->dat[0].i, 0, 255);
				col_bg[1] = limint(tmp->dat[1].i, 0, 255);
				col_bg[2] = limint(tmp->dat[2].i, 0, 255);
			}
			else if (ENTRY_NAME("scope_color") && tmp->type==E_RGB)
			{
				col_scope[0] = limint(tmp->dat[0].i, 0, 255);
				col_scope[1] = limint(tmp->dat[1].i, 0, 255);
				col_scope[2] = limint(tmp->dat[2].i, 0, 255);
			}
			else if (ENTRY_NAME("scope_low_grad_color") && tmp->type==E_RGB)
			{
				col_scope_low[0] = limint(tmp->dat[0].i, 0, 255);
				col_scope_low[1] = limint(tmp->dat[1].i, 0, 255);
				col_scope_low[2] = limint(tmp->dat[2].i, 0, 255);
			}
			else if (ENTRY_NAME("scope_high_grad_color") && tmp->type==E_RGB)
			{
				col_scope_high[0] = limint(tmp->dat[0].i, 0, 255);
				col_scope_high[1] = limint(tmp->dat[1].i, 0, 255);
				col_scope_high[2] = limint(tmp->dat[2].i, 0, 255);
			}
			else if (ENTRY_NAME("buf_size") && tmp->type==E_INT)
				c_buf_size = limint(tmp->dat[0].i, 16, 4096*4);
				
			else if (ENTRY_NAME("noborder") && CONF_IS_BOOL)
				f_borderless = tmp->dat[0].i;
				
			else if (ENTRY_NAME("len_ms") && tmp->type==E_INT)
				ao_set_len = tmp->dat[0].i;
				
			else if (ENTRY_NAME("scope_grad") && CONF_IS_BOOL)
				c_scope_do_grad = tmp->dat[0].i;
				
			else if (ENTRY_NAME("screen_width") && tmp->type==E_INT)
				c_screen_w = limint(tmp->dat[0].i, 16, 4096*2);
				
			else if (ENTRY_NAME("screen_height") && tmp->type==E_INT)
				c_screen_h = limint(tmp->dat[0].i, 16, 4096*2);
				
			else if (ENTRY_NAME("always_on_top") && CONF_IS_BOOL)
				c_always_ontop = tmp->dat[0].i;
				
			else if (ENTRY_NAME("mono") && CONF_IS_BOOL)
				mono = tmp->dat[0].i;
				
			else if (ENTRY_NAME("show_track") && CONF_IS_BOOL)
				info_disp[0] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_author") && CONF_IS_BOOL)
				info_disp[1] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_game") && CONF_IS_BOOL)
				info_disp[2] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_system") && CONF_IS_BOOL)
				info_disp[3] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_year") && CONF_IS_BOOL)
				info_disp[4] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_notes") && CONF_IS_BOOL)
				info_disp[5] = (tmp->dat[0].i) ? 1:0;
				
			else if (ENTRY_NAME("show_chips") && CONF_IS_BOOL)
				info_disp[6] = (tmp->dat[0].i) ? 1:0;
			
		}
		
		
		tmp = tmp->next;
	}
	
	if (o!=0)
		free_cfg_entries(o);
}

void dump_scope_buf()
{
	int i, j;
	float bufj;
	
	if (scope_bufsiz<=0)
	{
		for (i=0;i<SCOPEWIDTH;i++)
			scope[i] = 0;
	}
	else
	{
		if (scope_bufsiz>scope_dump_max)
			scope_bufsiz = scope_dump_max;
			
		bufj = (float) scope_bufsiz / SCOPEWIDTH;
		
		bufj *= scope_view;
		
		for (i=0;i<SCOPEWIDTH;i++)
		{
			j = bufj*i;
			
			j = limint(j, 0, scope_bufsiz - 1);
			
			scope[i] = scope_buf[j];
		}
	}
	
	
	scope_bufsiz = 0;
	return;
}


char *str_prepend(char *s, char *pre)
{
	if (!s || !pre)
		return 0;
	
	char * nstr;
	int i,j;
	
	nstr = (char*) malloc( strlen(s)+ strlen(pre) + 1);
	
	sprintf(nstr, "%s%s",pre,s);
	
	
	return nstr;
}

char *get_fn_only(char *path)
{
	char *t = &path[0];
	char *f = 0;
	
	while (*t!='\0')
	{
		if (*(t-1)=='\\' || *(t-1)=='/')
			f=t;
		
		t++;
	}
	
	return f;
}

int cheap_is_dir(char *path)
{
	int l;
	
	if (path==0)
		return -1;
	
	if (path[strlen(path)-1]=='/' || path[strlen(path)-1]=='\\')
		return 1;
		
	return 0;
}

void reset_u_buf()
{
	int i;
	
	u_buf_fill_start = 0;
	u_buf_fill = 0;
	
	for (i=0;i<u_size;i++)
		u_buf[i] = 0;
}

void update(const Uint8 * buf, int size)
{
	/* as the buffer execute_psf fills update with is always
	 * fixed, we have another buffer to allow different buffer sizes.
	 */
	int i,j, tmp, tmp1, goal, over,newsz,got;
	signed short stmp, stmp1;
	Uint8 *ctmp;
	
	if (buf==0 || size==0)
	{
		/* if function calling update returns 0, 0, halt playback.
		 * 
		 * (WATCH THIS)
		 */
		play_stat = M_DO_STOP;
		
		return;
	}
	
	/* resize u_buf if update thows too much data */
	if (size > (u_size-(u_buf_fill*2)))
	{
		newsz = (size * 2 + (size/2)) + (u_buf_fill*2);
		
		#if DEBUG_MAIN
		printf("note: buffer resize from %d to %d\n", (u_size), newsz);
		#endif
		ctmp = ( Uint8 *) malloc(sizeof ( Uint8 ) * newsz);
		
		
		j=u_buf_fill_start;
		i=0;
		goal = (u_buf_fill + u_buf_fill_start) % u_size;
		while (j!=goal)
		{
			ctmp[i] = u_buf[j];
			
			j++;
			j%=u_size;
			
			i++;
		}
		
		free(u_buf);
		u_buf = ctmp;
		ctmp=0;
		u_size = newsz;
		u_buf_fill_start = 0;
	}
	
	
	j=u_buf_fill_start;
	i=0;
	goal = u_buf_fill_start + (STEREO_FILL_SIZE);
	goal %= u_size;
	
	
	j+=u_buf_fill;
	j%=u_size;
		
	
	if (mono==0) /* STREREO */
		while (i < size)
		{
			u_buf[j] = buf[i];
			
			u_buf_fill++;
				
			
			j++;
			j%=u_size;
			
			i++;
		}
	else /* MONO  (assuming 4-byte 2ch signed short le )*/
		while (i < size)
		{
			if (i%4==0)
			{
				stmp = (buf[i+1]<<8) + (buf[i] & 0xff);
				stmp1 = (buf[i+3]<<8) + (buf[i+2] & 0xff);
				
				tmp = ((int)stmp + (int)stmp1) / 2;
				
				u_buf[j % u_size] = tmp & 0xff;
				u_buf[j+1 % u_size] = (tmp>>8) & 0xff;
				
				u_buf[j+2 % u_size] = tmp & 0xff;
				u_buf[j+3 % u_size] = (tmp>>8) & 0xff;
				
			
				u_buf_fill+=4;
					
				
				j+=4;
				j%=u_size;
				
			}
			
			i++;
		}
		
		
	
	return;
}

void fill_audio(void *udata, Uint8 *stream, int len)
{
	int i, j;
	
	if (play_stat==M_PLAY)
	{
		while (u_buf_fill < len && play_stat == M_PLAY)
		{
			if (file_execute( (void (*)(const void *, int)) update) == AO_FAIL)
				play_stat = M_DO_STOP;
		}
			
		if (play_stat != M_PLAY)
		{
			reset_u_buf();
			return;
		}
		
		i=0;
		j=u_buf_fill_start;
		
		while (i < len)
		{
			stream[i] = u_buf[j]; /* fill stream */
			
			
			i++;
			
			if (j%4==0 && j+1 < u_size && scope_bufsiz < 1024)
			{
				scope_buf[scope_bufsiz] = (u_buf[j+1]<<8) + (u_buf[j]);
				scope_bufsiz++;
			}
			
			j++;
			j%=u_size;
			
			u_buf_fill--;
		}
		
		u_buf_fill_start += len;
		u_buf_fill_start %= u_size;
		
			
	}
	
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

void free_scope()
{
	free(scope);
}

void init_scope()
{
	scope = malloc(sizeof(int16_t) * SCOPEWIDTH);
}

void clear_scope()
{
	int i;
	
	for (i=0;i<SCOPEWIDTH;i++)
		scope[i] = 0;
		
	for (i=0;i<scope_bufsiz;i++)
		scope_buf[i]=0;
}


void update_scope(SDL_Surface *in)
{
	
	int i, x, y, j;
	
	float fy, gfy;
	
	int *col_grad_tmp;
	
	static int grgb[3];
	
	if (pw_init(in))
	{
		pw_set_rgb(BG_COLOR);
		
		pw_fill(in);
		
		pw_set_rgb(col_scope[0], col_scope[1], col_scope[2]);
		
		for (i=0;i<SCOPEWIDTH && i < SCREENWIDTH;i++)
		{
			x=i;
			
			fy=(float) scope[i] / (0xffff/2);

			if (c_scope_do_grad)
			{	/* color depends on y */
				gfy = (fy<0) ? -fy * 1.5 : fy * 1.5;
				col_grad_tmp = (fy<0) ? col_scope_low : col_scope_high;
				
				for (j=0;j<3;j++)
					grgb[j] =
						limint (
							col_scope[j] + (int) ((float)
								(col_scope_low[j]-col_scope[j]) * gfy)
							, 0, 255);
				
				pw_set_rgb(grgb[0], grgb[1], grgb[2]);
			}
			
			y = (int) (fy * (SCREENHEIGHT/2));
			y += (SCREENHEIGHT/2);
			y = y<0? 0:y;
			y = y>(SCREENHEIGHT-1)?(SCREENHEIGHT-1):y;
			
			pw_set(in, x, y);
			
			pw_set(in, x, y+1);
		}
	}
	
}

void update_ao_chdisp(SDL_Surface *in)
{
	
	int i, x, y, to_y, minmax;
	
	float fy;
	
	
	if (pw_init(in))
	{		
		pw_set_rgb(SCOPE_COLOR);
		
		for (i=0;i<(ao_chan_disp_nchannels*2);i++)
		{
			fy = 0.0;
			
			if (ao_chan_disp_max[i] > ao_chan_disp_min[i])
			{
				/*fy=(float) ( ao_chan_disp[i] ) / (0xfff);*/
				minmax = ao_chan_disp_max[i]-ao_chan_disp_min[i];
				
				fy=(float) ( ao_chan_disp[i] ) / (minmax);
			}
			
			
			
			fy=fy>1.0?1.0:fy;
			fy=fy<0.0?0.0:fy;
			
			
			x = SCREENWIDTH - 10 - (ao_chan_disp_nchannels*2*2) +  ((i) + (i/2 * 2)  );
			y = SCREENHEIGHT - 10;
			to_y = y - (int) (fy*16);
			
			y+=1;
			
			if (ao_channel_enable[i/2] == 0)
				to_y = y + 3;
							
			while (y > to_y)
			{
				pw_set(in, x, y);
				y--;
			}
		
			while (y < to_y)
			{
				pw_set(in, x, y);
				y++;
			}
		}
	}
	
}

void update_ao_ch_flag_disp(SDL_Surface *in, int md)
{
	
	int i, j, x, y, tmp;
	
	float fy;
	
	static char tmpstr[32];
	
	SDL_Color tcol_a;
	
	sdl_set_col(&tcol_a, SCOPE_COLOR, 0);
	
	
	if (pw_init(in))
	{		
		pw_set_rgb(SCOPE_COLOR);
		
		for (i=0;i<(ao_chan_disp_nchannels);i++)
		{
			tmp = ao_chan_flag_disp[i];
			y=SCREENHEIGHT - (ao_chan_disp_nchannels*2) - 4 + (i*2);
			
			
			y = SCREENHEIGHT - 10;
			x = SCREENWIDTH - 10 - (ao_chan_disp_nchannels*2*2) +  (i + (i * 3))  ;
			
			
			if (md==1)
			{
				if (tmp & (1))
					pw_set(in, x, y+4);
				if (tmp & (2))
					pw_set(in, x, y+5);
				if (tmp & (4))
					pw_set(in, x, y+6);
			}
			
			
			if (md==2)
			{
				pw_set_rgb(tmp&0xff,tmp>>8&0xff,tmp>>16&0xff);
				pw_set(in, x, y+4);
				pw_set(in, x+1, y+4);
				pw_set(in, x, y+5);
				pw_set(in, x+1, y+5);
			}
			
			pw_set_rgb(SCOPE_COLOR);
		}
	}
}

void render_text(
	SDL_Surface* dst,
	char *str,
	SDL_Color *col_a,
	SDL_Color *col_b,
	TTF_Font *font,
	int x,
	int y,
	int bold,
	int maxwidth,
	int tick)
{
	static SDL_Surface* text;
	int maxtks, tkframe, shiftx, toobig=0;
	SDL_Rect tclip,tr;
	tclip.x=0;
	tclip.y=0;
	tclip.h=256;
	
	if (!f_render_font)
		return;
	
	if (str!=0 && str[0] == '\0')
		return;
	
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	text = TTF_RenderUTF8_Solid(font, str, *col_b);
	
	
	tclip.h=text->h;
	tclip.w=maxwidth;
	
	if (maxwidth < text->w)
	{
		maxtks=((text->w)+TEXT_SCROLL_GAP)*2;
		tkframe = tick % maxtks;
		
		shiftx = (tkframe/2);
		
		toobig=1;
	}
	
	
	if (toobig)
		tclip.x += shiftx;
	
	tr.x=x-1;
	tr.y=y;
	SDL_BlitSurface(text, &tclip, dst, &tr);
	SDL_FreeSurface(text);
	
	text = TTF_RenderUTF8_Solid(font, str, *col_a);
	tr.x=x;
	SDL_BlitSurface(text, &tclip, dst, &tr);
	
	
	if (bold==1)
	{
		tr.x+=1;
		SDL_BlitSurface(text, &tclip, dst, &tr);
	}
	
	SDL_FreeSurface(text);
	
	if (toobig)
	{
		tclip.x=0;
		x+=(maxtks/2)-shiftx;
		tclip.w-=(maxtks/2)-shiftx;
		
		tr.x=x-1;
		text = TTF_RenderUTF8_Solid(font, str, *col_b);
		SDL_BlitSurface(text, &tclip, dst, &tr);
		SDL_FreeSurface(text);
		
		text = TTF_RenderUTF8_Solid(font, str, *col_a);
		tr.x=x;
		SDL_BlitSurface(text, &tclip, dst, &tr);
		
		if (bold==1)
		{
			tr.x+=1;
			SDL_BlitSurface(text, &tclip, dst, &tr);
		}
		
		SDL_FreeSurface(text);
	}
}

void sdl_set_col(SDL_Color *c, int r,int  g,int  b,int  a)
{
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
}

void u_buf_init()
{
	u_buf = ( Uint8 *) malloc(sizeof ( Uint8 ) * u_size);
}

char * get_ext(char * fn)
{
	char * e;
	
	e = (fn + strlen(fn) - 1);
	
	while (e > fn && *e != '.')
		e--;
		
	if (*e=='.')
		e++;
	
	return e;
}



int get_file_type(char * fn)
{
	FILE *fp;
	gzFile *gz_fp;
	char *ext, ch, i, is;
	
	if (!fn)
		return -1;
		

	ext = get_ext(fn);
	
	if (!(fp = fopen(fn, "r")))
		return -1;
		
	fclose(fp);
		
	if (!strcmp(ext,"psf") || !strcmp(ext,"PSF"))
		return F_PSF;
		
	if (!strcmp(ext,"psf2") || !strcmp(ext,"PSF2"))
		return F_PSF2;
		
	if (!strcmp(ext,"usf") || !strcmp(ext,"USF"))
		return F_USF;
		
	if (!strcmp(ext,"vgm") || !strcmp(ext,"VGM"))
		return F_VGM;
	
	if (!strcmp(ext,"vgz")||!strcmp(ext,"VGZ"))
		return F_VGM;
		
	if (!strcmp(ext,"sid")||!strcmp(ext,"SID"))
		return F_SID;
	
	
	return F_UNKNOWN;
	
}

void print_f_type(int type)
{
	switch (type)
	{
	case F_PSF:
		printf("psf\n");
		return;
	case F_PSF2:
		printf("psf2\n");
		return;
	case F_USF:
		printf("usf\n");
		return;
	case F_VGM:
		printf("vgm\n");
		return;
	default:
		printf("unknown\n");
	}
}

void experimental_sample_filter(int key)
{
	switch(key)
	{
	case SDLK_0:
		if (ao_sample_limit[0]==0)
			ao_sample_limit[0]=1;
		else
			ao_sample_limit[0]=0;
		break;
	case SDLK_1:ao_sample_do[0] = ao_sample_do[0] ? 0 : 1;break;
	case SDLK_2:ao_sample_do[1] = ao_sample_do[1] ? 0 : 1;break;
	case SDLK_3:ao_sample_do[2] = ao_sample_do[2] ? 0 : 1;break;
	case SDLK_4:ao_sample_do[3] = ao_sample_do[3] ? 0 : 1;break;
	case SDLK_5:ao_sample_do[4] = ao_sample_do[4] ? 0 : 1;break;
	case SDLK_6:ao_sample_do[5] = ao_sample_do[5] ? 0 : 1;break;
	case SDLK_7:ao_sample_do[6] = ao_sample_do[6] ? 0 : 1;break;
	case SDLK_8:ao_sample_do[7] = ao_sample_do[7] ? 0 : 1;break;
	case SDLK_9:ao_sample_do[8] = ao_sample_do[8] ? 0 : 1;break;
	case SDLK_q:ao_sample_do[9] = ao_sample_do[9] ? 0 : 1;break;
	case SDLK_w:ao_sample_do[10] = ao_sample_do[10] ? 0 : 1;break;
	case SDLK_e:ao_sample_do[11] = ao_sample_do[11] ? 0 : 1;break;
	case SDLK_r:ao_sample_do[12] = ao_sample_do[12] ? 0 : 1;break;
	case SDLK_t:ao_sample_do[13] = ao_sample_do[13] ? 0 : 1;break;
	case SDLK_y:ao_sample_do[14] = ao_sample_do[14] ? 0 : 1;break;
	case SDLK_u:ao_sample_do[15] = ao_sample_do[15] ? 0 : 1;break;
	case SDLK_i:ao_sample_do[16] = ao_sample_do[16] ? 0 : 1;break;
	case SDLK_o:ao_sample_do[17] = ao_sample_do[17] ? 0 : 1;break;
	case SDLK_p:ao_sample_do[18] = ao_sample_do[18] ? 0 : 1;break;
	default:break;
	}	
}



void listdir_flist(struct flist_base *b, const char *name, int level)
{
	struct dirent **namelist;
	char pathcat[1024], *ctmp;
	int i, n;


    n = scandir(name, &namelist, 0, alphasort);
    if (n < 0)
        return;
    else
	{

		for (i = 0; i < n; i++)
        {
			if (namelist[i]->d_type != DT_DIR)
            {
				ctmp = filename_build(name, namelist[i]->d_name);
				if (recon_file(ctmp)==1)
					add_flist_item(b, ctmp);
				else
					free(ctmp);
			}
			
			free(namelist[i]);
		}
	}
	free(namelist);
}

int recon_file(char* fn)
{
	int a;
	
	if (!fn)
		return 0;
		
	a = get_file_type(fn);
		
	if (a == F_UNKNOWN || a == -1)
		return 0;
	
	return 1;
}
