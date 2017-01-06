

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pthread.h>
#include <glib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>

#include <gtk/gtk.h>



#include <zlib.h>

#include "ao.h"

#include "conf.h"

#include "psf/plugin.h"

#include "vgm/plugin.h"

#include "sid/plugin.h"

#include "gme/plugin.h"

#include "gsf/plugin.h"

#include "usf/usfpluginout.h"

#include "dsf/dsf_plugin.h"

#include "dumb/src/plugin.h"


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

static int f_play_file_choose = 0;

static int f_filter_surround = 0;

static int f_filter_hi_pass = 0;

static int f_filter_hi_pass_f0 = 100;
static float f_filter_hi_pass_q = 1.5;
static int f_filter_hi_pass_precision = 10;
static float f_filter_hi_pass_mix = 0.5;

static float f_boost=1.0;

char default_folder[256];

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
static int k_ctrl=0;

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
	F_SID,
	F_GME,
	F_GSF,
	F_DSF,
	F_QSF,
	F_SSF,
	F_2SF,
	F_DUH
};

int play_stat = M_STOPPED;


struct flist_base *flist;

struct key_comb
{
	int shift; /* boolean values */
	int ctrl;
	int alt;
	
	int key; /* SDL defined key syms. */
};

/*key_tran_str =
	{
		"escape",
		"space",
		"left",
		"right",
		"up",
		"down",
		"",
		
	}*/

struct key_comb kshortcuts[16];

enum
{
	KS_PREV_TRACK,
	KS_NEXT_TRACK,
	KS_FIRST_TRACK,
	KS_LAST_TRACK,
	KS_RELOAD_CONF,
	KS_TOGGLE_MONO,
	KS_PAUSE,
	KS_QUIT,
	KS_OPEN_FILE,
	KS_DELETE_CURRENT_ITEM,
	KS_DELETE_ALL_ITEMS,
	KS_END
};

void keys_reset()
{
	int i;
	
	for (i=0;i<KS_END;i++)
	{
		kshortcuts[i].shift = 0;
		kshortcuts[i].ctrl = 0;
		kshortcuts[i].alt = 0;
	}
	
	kshortcuts[KS_PREV_TRACK].key = SDLK_LEFT;
	kshortcuts[KS_NEXT_TRACK].key = SDLK_RIGHT;
	kshortcuts[KS_FIRST_TRACK].key = SDLK_DOWN;
	kshortcuts[KS_LAST_TRACK].key = SDLK_UP;
	kshortcuts[KS_RELOAD_CONF].key = SDLK_c;
	kshortcuts[KS_TOGGLE_MONO].key = SDLK_m;
	kshortcuts[KS_PAUSE].key = SDLK_SPACE;
	kshortcuts[KS_QUIT].key = SDLK_ESCAPE;
	kshortcuts[KS_OPEN_FILE].key = SDLK_f;
	kshortcuts[KS_DELETE_CURRENT_ITEM].key = SDLK_x;
	kshortcuts[KS_DELETE_ALL_ITEMS].key = SDLK_x;
	
	kshortcuts[KS_DELETE_ALL_ITEMS].shift = 1;
}


GtkWidget *
create_filechooser_dialog(char *init_path, GtkFileChooserAction action)
{
  GtkWidget *wdg = NULL;

  switch (action) {
    case GTK_FILE_CHOOSER_ACTION_SAVE:
      wdg = gtk_file_chooser_dialog_new("Save file", NULL, action,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_OK,
        NULL);
      break;

    case GTK_FILE_CHOOSER_ACTION_OPEN:
      wdg = gtk_file_chooser_dialog_new("Open file", NULL, action,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Open", GTK_RESPONSE_OK,
        NULL);
      break;

    case GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:
    case GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER:
      break;
  }
  
  gtk_file_chooser_set_select_multiple((GtkFileChooser *)wdg, TRUE);
  
  gtk_file_chooser_set_current_folder ((GtkFileChooser *)wdg, init_path);

  return wdg;
}

static void (*file_close)(void) = 0;
static int (*file_execute)(void (*update )(const void *, int)) = 0;
static int (*file_open)(char *) = 0;

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


/* this is so wrong */
enum
{
	GTK_SLAVE_OFF,
	GTK_SLAVE_STANDBY,
	GTK_SLAVE_KILL
};

int gtk_slave_state = GTK_SLAVE_OFF;
GtkWidget *wdg_slave;
char gtk_choose_fn[1024] = "./";
pthread_t gtk_slave_thread;
int gtk_slave_set_file = -1;




void *do_gtk_dialog_slave(void* thread_id)
{
	int i;
	GSList *tmp_list;
	
	gtk_slave_set_file = -1;
	
	gtk_slave_state = GTK_SLAVE_STANDBY;

	gtk_init(0, 0);
	
	wdg_slave =
		create_filechooser_dialog(
			gtk_choose_fn,
			GTK_FILE_CHOOSER_ACTION_OPEN
			);
	
	
	
	gtk_widget_map(wdg_slave);
	
	if (gtk_dialog_run(GTK_DIALOG(wdg_slave)) == GTK_RESPONSE_OK)
	{
		tmp_list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(wdg_slave));
		
		if (tmp_list != 0)
		{
			for (i=0;i<g_slist_length(tmp_list);i++)
			{
				if (i==0 && f_play_file_choose)
					gtk_slave_set_file = flist->len;
				
				add_flist_item(
					flist,
					(char *) g_slist_nth_data(tmp_list, i) );
			}
			
			g_slist_free(tmp_list);
		}
		
		tmp_list = 0;
		
	}
	
	gtk_choose_fn[0] = '\0'; /* watch this. */
	strcpy(
		gtk_choose_fn,
		gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(wdg_slave))
			);
			
	gtk_widget_unmap(wdg_slave);
	
	gtk_slave_state = GTK_SLAVE_KILL;
	
	gtk_dialog_run(GTK_DIALOG(wdg_slave));
	
	gtk_slave_state = GTK_SLAVE_OFF;
	
	gtk_widget_destroy(wdg_slave);
	
	pthread_exit(0);
	
	
}

void gtk_slave_file_chooser_open()
{
	if (gtk_slave_state==GTK_SLAVE_OFF)
		pthread_create(&gtk_slave_thread, 0, do_gtk_dialog_slave, (void *) 0 );
}


void gtk_slave_file_check_kill()
{
	static int i = 16;
	
	if (gtk_slave_state == GTK_SLAVE_KILL)
	{
		i--;
		
		if (!i)  /* file chooser meets its demise here */
			gtk_dialog_response(GTK_DIALOG(wdg_slave), 0);
			
	}
	else
		i = 16;
		
}			
					

	
int main(int argc, char *argv[])
{
	int run, i, fcur,
		key, tickdelay,
		tick, rtmpy;
	SDL_Event e;
	char tmpstr[256];
	char tmpstr1[64];
	char chtrack, *ctmp;	
	
	/* reset keyboard shortcuts to default */
	keys_reset();
	
	load_conf(CONFIG_FNAME);
	
	SDL_AudioSpec wanted;
	SDL_Window *win;
	SDL_Surface *screen;
	SDL_Color tcol_a;
	SDL_Color tcol_b;
	SDL_Surface* tsurf;
	SDL_Rect tr;
	
	FILE *fp;

	/* set sdl color structs */
	sdl_set_col(&tcol_a, TEXT_COLOR, 0);
	sdl_set_col(&tcol_b, TEXT_BG_COLOR, 0);
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	/* init audio buffer */
	u_buf_init();
	
	/* init and clear scope visual */
	init_scope();
	clear_scope();
	
	TTF_Init();
	
	/* load our font */
    TTF_Font *font = TTF_OpenFont(TTF_FONT, 11);
    
    if (!font)
    {
		fprintf(stderr,
			"err: could't find \"%s\".\n no text will be rendered.\n",
			TTF_FONT);
		f_render_font=0;
	}
	
	/* load files from arguments and given directories into a file list. */
	flist = flist_init();
	for (i=1;i<argc;i++)
	{
		if (is_dir(argv[i]))
			listdir_flist(flist, argv[i], 0);
			
		else if (recon_file(argv[i])==1)
			add_flist_item(flist, argv[i]);
			
	}
	
	/* set gtk file chooser directory */
	strcpy(gtk_choose_fn, default_folder);
	
	/* set action if opened with no files in arguments */
	if (flist->len == 0)
	{
		play_stat = M_NOFILE;
		
		gtk_slave_file_chooser_open();
		
	}
	
	/* set variables to keep track of current file/track within file. */
	fcur = 0;
	ao_track_select = 0;
	ao_track_max = -1;
	
	reset_chan_disp();
	
	win = SDL_CreateWindow(WIN_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREENWIDTH, SCREENHEIGHT,
		(SDL_WINDOW_BORDERLESS * f_borderless)
		);
		
	screen = SDL_GetWindowSurface(win);
	
	
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
    
    
    if (play_stat != M_NOFILE)
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
				k_ctrl = e.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL);
				
				
				#define KS_IF_MATCH \
					if (!(kshortcuts[i].key == e.key.keysym.sym && \
					 ((kshortcuts[i].shift != 0) ? (k_shift!=0):1) && \
					 ((kshortcuts[i].ctrl != 0) ? (k_ctrl!=0):1)))\
						break;
				
				#define KS_CHANGE_TRACK_DOAFTER \
					if (play_stat == M_PLAY) \
							play_stat = M_RELOAD; \
						else if (play_stat == M_PAUSE) \
							play_stat = M_RELOAD_IDLE; \
						
						if (play_stat == M_STOPPED || play_stat == M_ERR) \
							play_stat = M_LOAD;
				
				#define KS_DEL_TRACK_DOAFTER \
					play_stat = M_RELOAD; \
					ao_track_select = 0; \
					ao_track_max = -1; \
					if (flist->len == 0) \
					{ \
						clear_tags(); \
						SDL_PauseAudio(1); \
						reset_u_buf(); \
						reset_chan_disp(); \
						clear_scope(); \
						if (file_close != 0) \
							file_close(); \
						fcur=0; \
						play_stat = M_NOFILE; \
					} \
					else if (fcur >= flist->len) \
						fcur = flist->len - 1;
				
				for (i=0;i<KS_END;i++)
					switch(i)
					{
					case KS_PREV_TRACK:
						KS_IF_MATCH
						if (ao_track_max == -1 && fcur > 0)
							fcur--;
						else
						{
							if (ao_track_select > 0)
								ao_track_select--;
							else if (fcur>0)
							{
								fcur--;
								ao_track_select=0xffff;
								/* so last track of previous
								 * file will be selected
								 */
							}
						}
						KS_CHANGE_TRACK_DOAFTER
						break;
						
					case KS_NEXT_TRACK:
						KS_IF_MATCH
						if (ao_track_max == -1 && fcur<(flist->len-1)) 
							fcur++;
						else
						{
							if (ao_track_select < ao_track_max)
								ao_track_select++;
							else if (fcur<(flist->len-1))
							{
								fcur++;
								ao_track_select = 0;
							}
						}
						KS_CHANGE_TRACK_DOAFTER
						break;
						
					case KS_FIRST_TRACK:
						KS_IF_MATCH
						if (ao_track_max == -1)
							fcur = 0;
						else
						{
							if (ao_track_select > 0)
								ao_track_select = 0;
							else
								fcur = 0;
						}
						KS_CHANGE_TRACK_DOAFTER
						break;
						
					case KS_LAST_TRACK:
						KS_IF_MATCH
						if (ao_track_max == -1)
							fcur = flist->len-1;
						else
						{
							if (ao_track_select < ao_track_max)
								ao_track_select = ao_track_max;
							else
								fcur = flist->len-1;
						}
						KS_CHANGE_TRACK_DOAFTER
						break;
						
					case KS_RELOAD_CONF:
						KS_IF_MATCH
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
						
					case KS_TOGGLE_MONO:
						KS_IF_MATCH
						mono = (mono == 1) ? 0 : 1;
						break;
						
					case KS_PAUSE:
						KS_IF_MATCH
						if (play_stat == M_STOPPED || play_stat == M_ERR)
							play_stat = M_LOAD;
						else if (play_stat == M_PAUSE)
							play_stat = M_PLAY;
						else if (play_stat == M_PLAY)
							play_stat = M_PAUSE;
						break;
						
					case KS_QUIT:
						KS_IF_MATCH
						run = 0;
						break;
						
					case KS_OPEN_FILE:
						KS_IF_MATCH
						gtk_slave_file_chooser_open();
						break;
						
					case KS_DELETE_CURRENT_ITEM:
						KS_IF_MATCH
						
						del_flist_idx(flist, fcur);
						
						KS_DEL_TRACK_DOAFTER
						break;
						
					case KS_DELETE_ALL_ITEMS:
						KS_IF_MATCH
						
						while (flist->len > 0)
							del_flist_idx(flist, 0);
						
						KS_DEL_TRACK_DOAFTER
						break;
					}
				
				break;
			default:break;
			}
		}
		
		gtk_slave_file_check_kill();
		
		if (gtk_slave_set_file != -1)
		{
			fcur = gtk_slave_set_file;
			play_stat = M_RELOAD;
			
			ao_track_select = -1;
			
			gtk_slave_set_file = -1;
		}
			
		
		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, BG_COLOR));
		
		
		if (play_stat == M_PLAY)
			dump_scope_buf();
			
		
		/* display scope */
		if (f_enable_scope_disp)
			update_scope(screen);
			
		mix_chan_disp_flush();
		
		/* display individual channels. */
		if (f_enable_channel_disp)
		{
			update_ao_chdisp_newer(screen);
			//update_ao_ch_flag_disp(screen, 2);
		}
		
		
		rtmpy = 8;

		for (i=0;i<7;i++)
		{
			if (info_disp[i])
			{
				switch (i)
				{
				case D_TRACK:
					ctmp = (play_stat == M_ERR && flist->len!=0) ?
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
			
			reset_u_buf();
			reset_chan_disp();
			dump_scope_buf();
			
			if (file_close != 0)
				file_close();
			
			if (play_stat==M_DO_ERR_STOP)
				play_stat = M_ERR;
			else
				play_stat = M_STOPPED;
			break;
		
		case M_RELOAD_IDLE:
		case M_RELOAD:
			clear_tags();
			
			SDL_PauseAudio(1);
			
			reset_u_buf();
		
			if (file_close != 0)
				file_close(); /* segues into M_LOAD on purpose. */
			
		case M_LOAD:
		
			SDL_PauseAudio(1);
			
			reset_u_buf();
			
			reset_chan_disp();
			
			ao_track_max = -1;
			
			if (get_file_type(get_flist_idx(flist,fcur)) == F_PSF)
			{
				file_close = &close_psf_file;
				file_execute = &psf_execute;
				file_open = &load_psf_file;
				
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_PSF2)
			{
				file_close = &close_psf2_file;
				file_execute = &psf2_execute;
				file_open = &load_psf2_file;
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
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_GME)
			{
				file_close = &gme_close;
				file_execute = &gme_execute;
				file_open = &gme_open;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_GSF)
			{
				file_close = &gsf_close;
				file_execute = &gsf_execute;
				file_open = &gsf_open;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_USF)
			{
				file_close = &usf_close;
				file_execute = &usf_execute;
				file_open = &usf_open;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_DSF)
			{
				file_close = &close_dsf_file;
				file_execute = &dsf_execute;
				file_open = &load_dsf_file;
			}
			else if (get_file_type(get_flist_idx(flist,fcur)) == F_DUH)
			{
				file_close = &pduh_close;
				file_execute = &pduh_execute;
				file_open = &pduh_open;
			}
			else
			{
				play_stat = M_ERR;
				
				file_close = file_execute = file_open = 0;
				
				break;
			}
			
			if (!file_open(get_flist_idx(flist,fcur)))
			{
				play_stat = M_ERR;
				clear_tags();
				SDL_PauseAudio(1);
				if (file_close!=0)
					file_close();
				
				file_close = 0;file_execute = 0;file_open = 0;
				
				break;
			}
			
			
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
		case M_NOFILE:
			if (flist->len != 0)
				play_stat = M_LOAD;
			break;
		}
		
		
		
		if (play_stat != M_NOFILE && flist->len != 0)
			sprintf(tmpstr,"(%d/%d)",fcur+1,flist->len);
		else
		{
			sprintf(tmpstr,"no files");
			
			/* also do this: */
			ao_track_max = -1;
		}
			
		
		if (ao_track_max != -1 || ao_track_max == 0)
		{
			sprintf(tmpstr1,"  track: (%d/%d)",ao_track_select+1,ao_track_max+1);
			strcat(tmpstr,tmpstr1);
		}
		
		render_text(
			screen,
			tmpstr,
			&tcol_a,
			&tcol_b,
			font,
			10, rtmpy,
			1,SCREENWIDTH,tick);
		
		/* play next track on stop if enabled */
		if (ao_play_next_on_stop && play_stat == M_STOPPED)
		{
			play_stat = M_LOAD;
			
			if (++fcur>=flist->len)
				fcur--;
			
		}
		
		
		SDL_UpdateWindowSurface(win);
		
		
        SDL_Delay(1000/FPS);
        
        if (tickdelay < 0)
			tick++;
        else
			tickdelay--;
	
	}
	SDL_CloseAudio();
	if (file_close != 0)
		file_close();
	free_scope();
	
	return 0;
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
		if (!strcmp("general",tmp->section))
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
				
			else if (ENTRY_NAME("play_file_choose") && CONF_IS_BOOL)
				f_play_file_choose = (tmp->dat[0].i) ? 1:0; 
			
			else if (ENTRY_NAME("default_folder") && tmp->type==E_STR  )
				strcpy(default_folder, tmp->dat[0].s);
			
			else if (ENTRY_NAME("surround") && CONF_IS_BOOL)
				f_filter_surround = (tmp->dat[0].i) ? 1:0;
			
			else if (ENTRY_NAME("hi_pass") && CONF_IS_BOOL)
				f_filter_hi_pass = (tmp->dat[0].i) ? 1:0;
			
			else if (ENTRY_NAME("hi_pass_f0") && tmp->type==E_INT)
				f_filter_hi_pass_f0 = tmp->dat[0].i;
				
			else if (ENTRY_NAME("hi_pass_precision") && tmp->type==E_INT)
				f_filter_hi_pass_precision = limint( tmp->dat[0].i, 2, 12);
			
			else if (ENTRY_NAME("hi_pass_q") && tmp->type==E_FLOAT)
				f_filter_hi_pass_q = tmp->dat[0].f;
			
			else if (ENTRY_NAME("hi_pass_mix") && tmp->type==E_FLOAT)
				f_filter_hi_pass_mix = tmp->dat[0].f;
				
			else if (ENTRY_NAME("volume_boost") && tmp->type==E_FLOAT)
				f_boost = tmp->dat[0].f;
			
			else if (ENTRY_NAME("show_scope") && CONF_IS_BOOL)
				f_enable_scope_disp = (tmp->dat[0].i) ? 1:0;
			
			else if (ENTRY_NAME("show_channels") && CONF_IS_BOOL)
				f_enable_channel_disp = (tmp->dat[0].i) ? 1:0;
			
			else if (ENTRY_NAME("play_next_on_stop") && CONF_IS_BOOL)
				ao_play_next_on_stop = (tmp->dat[0].i) ? 1:0;
			
			
		}
		/*
		else if (!strcmp("keyboard",tmp->section))
		{
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
			if (ENTRY_NAME("prev_track") && tmp->type==E_STR  )
				strcpy(c_ttf_font, tmp->dat[0].s);
		}*/
		
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
			ctmp[i++] = u_buf[j++];
			
			j%=u_size;
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
		

	while (i < size)
	{
		u_buf[j++] = buf[i++];
		
		u_buf_fill++;

		j%=u_size;
	}
	
	return;
}

void fill_audio(void *udata, Uint8 *stream, int len)
{
	int i, j, x0,x1,x2,x3,x4;
	int16_t tmpl, tmpr;
		
	float fs, f0, q, w0, a0, a1, a2, b0, b1, b2, alpha,
		fx0,fx1,fx2,fx3,fx4;
	
	
	#ifndef PI
	#define PI 3.14159265359
	#endif
	
	#define LIMIT16(a) \
				if (a < (-0xffff/2 + 16)) \
					a = (-0xffff/2 + 16); \
				if (a > (0xffff/2 - 16)) \
					a = (0xffff/2 - 16)
	
	
	/* filter vars */

	fs=44100; 
	f0 = (float) f_filter_hi_pass_f0; /* cut-off (or center) frequency in Hz */
	q = f_filter_hi_pass_q;/* filter Q */
	w0 = 2 * PI * f0 / fs;
	alpha = sin(w0) / (2 * q);
	a0 = 1 + alpha;
	a1 = -2 * cos(w0);
	a2 = 1 - alpha;
	b0 = (1 + cos(w0)) / 2;
	b1 = -(1 + cos(w0));
	b2 = (1 + cos(w0)) / 2;
	
		
	x0 = ((float) b0 / a0) * pow(2,f_filter_hi_pass_precision);
	x1 = ((float) b1 / a0) * pow(2,f_filter_hi_pass_precision);
	x2 = ((float) b2 / a0) * pow(2,f_filter_hi_pass_precision);
	x3 = ((float) a1 / a0) * pow(2,f_filter_hi_pass_precision);
	x4 = ((float) a2 / a0) * pow(2,f_filter_hi_pass_precision);
	
	fx0 = b0/a0;
	fx1 = b1/a0;
	fx2 = b2/a0;
	fx3 = a1/a0;
	fx4 = a2/a0;
	
	
	static int prevl0=0,
		prevl1=0,
		prevl2=0,
		prevr0=0,
		prevr1=0,
		prevr2=0,
		pprevl1=0,
		pprevl2=0,
		pprevr1=0,
		pprevr2=0;
	
	if (f_filter_hi_pass_mix > 1.0)
		f_filter_hi_pass_mix = 1.0;
	if (f_filter_hi_pass_mix < 0.0)
		f_filter_hi_pass_mix = 0.0;
	
	
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
		
		if (len % 4 == 0) /* must be two-channel signed shorts (4 bytes) */
			while (i < len) 
			{
				/* copy data from u_buf */
				tmpl = (u_buf[j+1] << 8) | (u_buf[j]);
				u_buf_fill-=2;  j+=2;  j%=u_size;
				
				tmpr = (u_buf[j+1] << 8) | (u_buf[j] );
				u_buf_fill-=2;  j+=2;  j%=u_size;
				
				/* filter it depending of set flags */
				
				LIMIT16(tmpl);
				LIMIT16(tmpr);
				
				
				
				if (f_filter_hi_pass)
				{
					prevl2 = prevl1; /* cycle pre-filter vars */
					prevl1 = prevl0;
					
					prevl0 = (int16_t) tmpl;
					
					prevr2 = prevr1;
					prevr1 = prevr0;
					prevr0 = (int16_t) tmpr;
					
					
					
					pprevl2 = pprevl1; /* cycle post-filter vars */
					pprevr2 = pprevr1;
					
					
					pprevl1 = (int) ((fx0 * prevl0)) +
							((fx1 * prevl1)) +
							((fx2 * prevl2)) -
							((fx3 * pprevl1)) -
							((fx4 * pprevl2));
					
					pprevr1 = (int) ((fx0 * prevr0)) +
							((fx1 * prevr1)) +
							((fx2 * prevr2)) -
							((fx3 * pprevr1)) -
							((fx4 * pprevr2));
					
					LIMIT16(pprevr1);
					LIMIT16(pprevl1);
					
					/* assign tmpl and r while applying mix variable (thrown in 2 for now) */
					tmpl = ((float) tmpl * (1.0-f_filter_hi_pass_mix)) + ((float) pprevl1 * 2 * f_filter_hi_pass_mix);
					tmpr = ((float) tmpr * (1.0-f_filter_hi_pass_mix)) + ((float) pprevr1 * 2 * f_filter_hi_pass_mix);
					
					
					LIMIT16(tmpl);
					LIMIT16(tmpr);
					
				}
				
				if (mono)
				{
					tmpl = (tmpl+tmpr)/2;
					tmpr = tmpl;
				}
				
				if (f_filter_surround)
					tmpr = -tmpr;
				
				
				if (f_boost	!= 1.0)
				{
					x0 = ((float) tmpl * f_boost);
					x1 = ((float) tmpr * f_boost);
					
					LIMIT16(x0);
					LIMIT16(x1);
					
					tmpl=x0;
					tmpr=x1;
				}
				
				
				
					
				
				
				/* copy to stream */
				stream[i++] = tmpl & 0xff;
				stream[i++] = (tmpl>>8) & 0xff;
				
				stream[i++] = tmpr & 0xff;
				stream[i++] = (tmpr>>8) & 0xff;
				
				if (scope_bufsiz < 1024)
				{
					if (!f_filter_surround)
						scope_buf[scope_bufsiz] = (tmpl + tmpr) / 2;
					else
						scope_buf[scope_bufsiz] = (tmpl + (-tmpr)) / 2;
					scope_bufsiz++;
				}
				
			}
		
		u_buf_fill_start += len;
		u_buf_fill_start %= u_size;
		
			
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
	
	int i, curchp, curchp_x, x, y, to_y, minmax, chpcnt;
	
	float fy;
	
	
	if (pw_init(in))
	{	
				
		pw_set_rgb(SCOPE_COLOR);
		
		chpcnt=0;
		
		for (i=0;i<4;i++)
			if (ao_channel_set_chip[i] != -1)
				chpcnt++;

		curchp_x = 0;
		

		for (curchp = 0; curchp < chpcnt;curchp++)
		{
			for (i=0;i<(ao_channel_nchannels[curchp]*2);i++)
			{
				fy = 0.0;
				
				
				if (ao_channel_max[(curchp * 128) + (i/2)] > ao_channel_min[(curchp * 128) + (i/2)])
				{
					minmax = ao_channel_max[(curchp * 128) + (i/2)]-ao_channel_min[(curchp * 128) + (i/2)];
					
					fy=(float) ( ao_channel_mix[curchp * 128 * 2 + i] ) / (minmax);
					
				}
				
				
				
				fy=fy>1.0?1.0:fy;
				fy=fy<0.0?0.0:fy;
				
				
				x = SCREENWIDTH - 10 - (ao_channel_nchannels[curchp] * 2 * 2 ) +  (i + (i/2 * 2)  ) -  curchp_x;
				
				
				y = SCREENHEIGHT - 10;
				to_y = y - (int) (fy*16);
				
				y+=1;
				
				/*if (ao_channel_enable[i/2] == 0)
					to_y = y + 3;*/
								
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
			
			curchp_x += (ao_channel_nchannels[curchp] * 2 * 2 ) +
					((ao_channel_nchannels[curchp]-1) +
						((ao_channel_nchannels[curchp]-1)/2) * 2) + 16;
			
		}
		
		
	}
	
}

void update_ao_chdisp_newer(SDL_Surface *in)
{
	
	int i, curchp, curchp_x, x, y, to_y, minmax, chpcnt, tmpx, tmpi,shifty;
	
	float fy;
	
	
	if (pw_init(in))
	{	
				
		pw_set_rgb(SCOPE_COLOR);
		
		chpcnt=0;
		
		for (i=0;i<4;i++)
			if (ao_channel_set_chip[i] != -1)
				chpcnt++;

		curchp_x = 0;
		
		tmpx=SCREENWIDTH-10;
		shifty=0;
		

		for (curchp = 0; curchp < chpcnt;curchp++)
		{
			for (i=ao_channel_nchannels[curchp]-1;i>=0;i--)
			{
				
				if (tmpx<SCREENWIDTH/2)
				{
					tmpx=SCREENWIDTH-10;
					shifty-=20;
				}
					
				
				fy = 0.0;
				
				tmpi = i * 2 + 1;
				
				if (ao_channel_max[(curchp * 128) + (tmpi/2)] > ao_channel_min[(curchp * 128) + (tmpi/2)])
				{
					minmax = ao_channel_max[(curchp * 128) + (tmpi/2)]-ao_channel_min[(curchp * 128) + (tmpi/2)];
					
					fy=(float) ( ao_channel_mix[curchp * 128 * 2 + tmpi] ) / (minmax);
					
				}
				
				
				
				fy=fy>1.0?1.0:fy;
				fy=fy<0.0?0.0:fy;
				
				
				
				
				y = SCREENHEIGHT - 10 + shifty;
				to_y = y - (int) (fy*16);
				
				y+=1;
				
				/*if (ao_channel_enable[i/2] == 0)
					to_y = y + 3;*/
								
				while (y > to_y)
				{
					pw_set(in, tmpx, y);
					y--;
				}
			
				while (y < to_y)
				{
					pw_set(in, tmpx, y);
					y++;
				}
				
				tmpx--;
				tmpi--;
				
				
				fy=0.0;
				
				if (ao_channel_max[(curchp * 128) + (tmpi/2)] > ao_channel_min[(curchp * 128) + (tmpi/2)])
				{
					minmax = ao_channel_max[(curchp * 128) + (tmpi/2)]-ao_channel_min[(curchp * 128) + (tmpi/2)];
					
					fy=(float) ( ao_channel_mix[curchp * 128 * 2 + tmpi] ) / (minmax);
					
				}
				
				
				
				fy=fy>1.0?1.0:fy;
				fy=fy<0.0?0.0:fy;
				
				
				
				
				y = SCREENHEIGHT - 10 + shifty;
				to_y = y - (int) (fy*16);
				
				y+=1;
				
				/*if (ao_channel_enable[i/2] == 0)
					to_y = y + 3;*/
								
				while (y > to_y)
				{
					pw_set(in, tmpx, y);
					y--;
				}
			
				while (y < to_y)
				{
					pw_set(in, tmpx, y);
					y++;
				}
				
				tmpx -=3;
				
				
			}
			
			tmpx-=10;
			
			curchp_x += (ao_channel_nchannels[curchp] * 2 * 2 ) +
					((ao_channel_nchannels[curchp]-1) +
						((ao_channel_nchannels[curchp]-1)/2) * 2) + 16;
			
		}
		
		
	}
	
}

void update_ao_ch_flag_disp(SDL_Surface *in, int md)
{
	
	int i, j, x, y, tmp, curchp, curchp_x, chpcnt;
	
	float fy;
	
	static char tmpstr[64];
	
	SDL_Color tcol_a;
	
	sdl_set_col(&tcol_a, SCOPE_COLOR, 0);
	
	
	if (pw_init(in))
	{		
		pw_set_rgb(SCOPE_COLOR);
		
		chpcnt=0;
		
		for (i=0;i<4;i++)
			if (ao_channel_set_chip[i] != -1)
				chpcnt++;

		curchp_x = 0;
		
		for (curchp = 0; curchp < chpcnt;curchp++)
		{
			for (i=0;i<(ao_channel_nchannels[curchp]);i++)
			{
		
				tmp = ao_chan_flag_disp[curchp * 128 + i];
				
				
				y = SCREENHEIGHT - 10;
				x = SCREENWIDTH - 10 - (ao_channel_nchannels[curchp]*2*2) +  (i + (i * 3)) - curchp_x  ;
				
				
				pw_set_rgb(tmp&0xff,tmp>>8&0xff,tmp>>16&0xff);
				pw_set(in, x, y+4);
				pw_set(in, x+1, y+4);
				pw_set(in, x, y+5);
				pw_set(in, x+1, y+5);
				
				
				pw_set_rgb(SCOPE_COLOR);
				
			}
			
			curchp_x += (ao_channel_nchannels[curchp]*2*2) +  ((ao_channel_nchannels[curchp]-1) +
						((ao_channel_nchannels[curchp]-1)/2) * 2) + 16;
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
		
	if (!strcmp_nocase(ext,"psf",-1) || !strcmp_nocase(ext,"minipsf",-1) )
		return F_PSF;
		
	if (!strcmp_nocase(ext,"psf2",-1))
		return F_PSF2;
		
	if (!strcmp_nocase(ext,"usf",-1) || !strcmp_nocase(ext,"miniusf",-1))
		return F_USF;
		
	if (!strcmp_nocase(ext,"vgm",-1) || 
		!strcmp_nocase(ext,"vgz",-1))
		return F_VGM;
		
	if (!strcmp_nocase(ext,"sid",-1))
		return F_SID;
	
	if (	!strcmp_nocase(ext,"ay",-1) ||
			!strcmp_nocase(ext,"gb",-1) ||
			!strcmp_nocase(ext,"gbs",-1) ||
			!strcmp_nocase(ext,"gym",-1) ||
			!strcmp_nocase(ext,"hes",-1) ||
			!strcmp_nocase(ext,"kss",-1) ||
			!strcmp_nocase(ext,"nsf",-1) ||
			!strcmp_nocase(ext,"nsfe",-1) ||
			!strcmp_nocase(ext,"sap",-1) ||
			!strcmp_nocase(ext,"spc",-1) /* not including vgm*/
			)
		return F_GME;
	
	if (!strcmp_nocase(ext,"gsf",-1) || !strcmp_nocase(ext,"minigsf",-1))
		return F_GSF;
	
	if (!strcmp_nocase(ext,"dsf",-1) || !strcmp_nocase(ext,"minidsf",-1))
		return F_DSF;
	
	if (!strcmp_nocase(ext,"qsf",-1) || !strcmp_nocase(ext,"miniqsf",-1))
		return F_QSF;
	
	if (!strcmp_nocase(ext,"ssf",-1) || !strcmp_nocase(ext,"minissf",-1))
		return F_SSF;
	
	if (!strcmp_nocase(ext,"2sf",-1) || !strcmp_nocase(ext,"mini2sf",-1))
		return F_2SF;
		
	if (	!strcmp_nocase(ext,"mod",-1) ||
			!strcmp_nocase(ext,"it",-1) ||
			!strcmp_nocase(ext,"xm",-1) ||
			!strcmp_nocase(ext,"s3m",-1) ||
			!strcmp_nocase(ext,"duh",-1)
			)
		return F_DUH;
	
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
	case F_SID:
		printf("sid\n");
		return;
	case F_GME:
		printf("gme\n");
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
