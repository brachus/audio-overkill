
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "eng_protos.h"

#include "ao.h"

#define DEBUG_MAIN (0)

#define SCREENWIDTH 320
#define SCREENHEIGHT 240
#define SCOPEWIDTH 320

#define TEXT_SCROLL_GAP 32



#define TEXT_COLOR 230,230,230
#define TEXT_BG_COLOR 0,0,200
#define BG_COLOR 0, 0, 0
#define SCOPE_COLOR 255, 255, 255


#define TEXT_COLOR 50,50,50
#define TEXT_BG_COLOR 230,230,250
#define BG_COLOR 255, 255, 255
#define SCOPE_COLOR 0, 0, 0


#define WIN_TITLE "Audio Overkill - PSF"

#define MAXCHAN 24

#define ABUFSIZ 1024


static int fok=AO_FAIL;
static int samples=44100 / 30;

static int16_t scope[SCOPEWIDTH];
static int16_t scope_buf[1024];
static int scope_bufsiz = -1;
static int scope_clear = 1;
static int scope_dump_max = 1024;

static float scope_view = 0.5;

static int mono = 0;


const int col_text[3] = {TEXT_COLOR};
const int col_text_bg[3] = {TEXT_BG_COLOR};
const int col_bg[3] = {BG_COLOR};
const int col_scope[3] = {SCOPE_COLOR};

static int channel_enable[MAXCHAN] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

static int u_size = 3000;
static Uint8  *u_buf = ( Uint8 *) malloc(sizeof ( Uint8 ) * u_size);
static int u_buf_fill_start = 0; /* where sdl begins to fill audio stream from */
static int u_buf_fill_size = ABUFSIZ * 2; /* portion in bytes out of u_buf that is taken by sdl audio fill function*/
static int u_buf_fill = 0; /* size in bytes filled from fill_start and on.*/



static SDL_PixelFormat *pw_fmt;
static int pw_bpp, pw_pitch;
static uint32_t pw_ucol;
static uint8_t *pw_pixel;


int pw_init(SDL_Surface *in)
{
	
	pw_fmt=in->format;
	pw_bpp = pw_fmt->BytesPerPixel;
	pw_pitch = in->pitch;
	
	return pw_bpp==4;
		
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
			
			j = j < 0 ? 0 : j;
			j = j >= scope_bufsiz ? scope_bufsiz-1 : j;
			
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

char *get_lib_dir(char *path)
{
	int i;
	char *slash, *npath, *ch;
	
	if (!path)
		return 0;
		
	slash = strrchr(path, '/');
	
	if (!slash)
		return 0;
		
	npath = (char *) malloc(strlen(path));
	
	ch = path;
	i=0;
	while ((ch) <= slash)
	{
		npath[i] = path[i];
		
		ch++;
		i++;
	}
	
	npath[i] = '\0';	
	
	return npath;
	
}

void update(const  Uint8  *buf, int size)
{
	
	int i, tmp, tmp1;
	signed short stmp, stmp1;
	
	if (mono==0) /* not mono, just copy to u_buf */
	{
		for (i=0;i<size;i++)
			u_buf[i] = buf[i];
	}
	else
	{
		for (i=0;i<size;i++) /* assuming two channels */
		{
			if (i%4==0) /* average the two signed shorts making up 2 channels.*/
			{
				
				
				stmp = (buf[i+1]<<8) + (buf[i] & 0xff);
				stmp1 = (buf[i+3]<<8) + (buf[i+2] & 0xff);
				
				tmp = ((int)stmp + (int)stmp1) / 2;
				
				u_buf[i] = tmp & 0xff;
				u_buf[i+1] = (tmp>>8) & 0xff;
				
				u_buf[i+2] = tmp & 0xff;
				u_buf[i+3] = (tmp>>8) & 0xff;
			}
			
			//u_buf[i] = buf[i];
		}
			
	}
		
	u_size = size;
	/*printf("%d, %d\n",*buf,size);*/
	return;
}

void fill_audio(void *udata, Uint8 *stream, int len)
{
	int i,j;
	if (fok==AO_SUCCESS)
	{
		psf_execute(update);
		
		i=0;
		
		while (i < len && i<u_size)
		{
			stream[i] = u_buf[i]; /* fill stream */
			i++;
			
			if (i%4==0 && i+1 < u_size && scope_bufsiz < 1024)
			{
				scope_buf[scope_bufsiz] = (u_buf[i+1]<<8) + (u_buf[i]); /*scope[j] = (u_buf[i+1]<<8) + (u_buf[i]);*/
				scope_bufsiz++;
			}
		}
	}
}

void reset_u_buf()
{
	u_buf_fill_start = 0;
	u_buf_fill = 0;
}

void update_alt(const  Uint8  *buf, int size)
{
	
	int i,j, tmp, tmp1, goal, over,newsz,got;
	signed short stmp, stmp1;
	Uint8 *ctmp;
	
	/* resize u_buf if update thows too much data */
	if (size > (u_size-(u_buf_fill*2)))
	{
		newsz = (size * 2 + (size/2)) + (u_buf_fill*2);
		printf("note: buffer resize from %d to %d\n", (u_size), newsz);
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
	goal = u_buf_fill_start + (u_buf_fill_size);
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

void fill_audio_alt(void *udata, Uint8 *stream, int len)
{
	int i,j,goal;
	
	if (fok==AO_SUCCESS)
	{
		while (u_buf_fill < len)
			psf_execute(update_alt);
		
		i=0;
		j=u_buf_fill_start;
		goal=j+len; /* should be same as j + u_buf_fill_size */
		goal%=u_size;
		
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

void load_psf_file(char *fn)
{
	char * ctmp;
	
	if (fok==AO_SUCCESS)
		return fok;
	
	int tmp, tlen;
	FILE *fp;
	u_int8_t *tbuf;
	printf("prep?\n");
	if (fn != 0)
	{
		ctmp=get_lib_dir(fn);
		printf("get_lib_dir ok\n");
		//ctmp=str_prepend(ctmp,"file://");
		printf("str_prepend ok\n");
		set_libdir(ctmp);
		printf("set_libdir ok\n");
	}
	else
		printf("no libdir?\n");
	printf("ok\n");
	
	if (!fn)
		fp = fopen("test.psf","rb");
	else
		fp = fopen(fn,"rb");
	fseek(fp, 0L, SEEK_END);
	tlen = ftell(fp);
	rewind(fp);
	
	tbuf = (u_int8_t *) malloc(sizeof(u_int8_t) * (tlen));
	
	tmp=0;
	while (tmp<tlen)
	{
		tbuf[tmp]=fgetc(fp);
		tmp++;
	}
	
	fok = psf_start(tbuf, tlen);
	
	return tmp;
}

void close_psf_file()
{
	if (fok==AO_SUCCESS)
		psf_stop();
	fok=0;
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
	
	int i, x, y;
	
	float fy;
	
	if (pw_init(in))
	{
		pw_set_rgb(BG_COLOR);
		
		pw_fill(in);
		
		pw_set_rgb(SCOPE_COLOR);
		
		for (i=0;i<SCOPEWIDTH;i++)
		{
			x=i;
			fy=(float) scope[i] / (0xffff/2);
			
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
	
	int i, x, y, to_y;
	
	float fy;
	
	
	if (pw_init(in))
	{		
		pw_set_rgb(SCOPE_COLOR);
		
		for (i=0;i<(24*2);i++)
		{
			fy=(float) ( ao_chan_disp[i] ) / (0xfff);
			
			fy=fy>1.0?1.0:fy;
			fy=fy<0.0?0.0:fy;
			
			
			x = SCREENWIDTH - 10 - (24*2*2) +  ((i) + (i/2 * 2)  );
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
	
	SDL_Color tcol_a = { SCOPE_COLOR, 0 };
	
	
	if (pw_init(in))
	{		
		pw_set_rgb(SCOPE_COLOR);
		
		for (i=0;i<(24);i++)
		{
			tmp = ao_chan_flag_disp[i];
			y=SCREENHEIGHT - (24*2) - 4 + (i*2);
			
			
			y = SCREENHEIGHT - 10;
			x = SCREENWIDTH - 10 - (24*2*2) +  (i + (i * 3))  ;
			
			
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
				
			/*printf("%d\n",tmp);*/
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
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	text = TTF_RenderText_Solid(font, str, *col_b);
	
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
	
	text = TTF_RenderText_Solid(font, str, *col_a);
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
		text = TTF_RenderText_Solid(font, str, *col_b);
		SDL_BlitSurface(text, &tclip, dst, &tr);
		SDL_FreeSurface(text);
		
		text = TTF_RenderText_Solid(font, str, *col_a);
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

int main(int argc, char *argv[])
{
	int run=1, i, fcnt, fcur, key, ispaused=0, newtrackcnt=20, tick;
	SDL_Event e;
	char *fn[1024];
	char tmpstr[256];
	char chtrack;
	SDL_AudioSpec wanted;
	SDL_Window *win;
	SDL_Surface *screen;
	SDL_Color tcol_a = { TEXT_COLOR, 0 };
	SDL_Color tcol_b = { TEXT_BG_COLOR, 0 };
	SDL_Surface* tsurf;
	SDL_Rect tr;
	
	FILE *fp;
	
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	
	TTF_Init();
	
	
    TTF_Font *font = TTF_OpenFont("ttf/gohufont-11.ttf", 11);
	
	fcnt=0;
	for (i=1;i<argc && fcnt<1024;i++)
	{
		fn[fcnt] = argv[i];
		
		if (cheap_is_dir(fn[fcnt]) == 1)
		{
			fprintf(stderr, "opening directories not supported\n");
			return 1;
		}
		
		fcnt++;
	}
	
	fcur = 0;
	
	win = SDL_CreateWindow(WIN_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREENWIDTH, SCREENHEIGHT,
		SDL_WINDOW_OPENGL);
		
	screen = SDL_GetWindowSurface(win);
	
	set_channel_enable(channel_enable);
	
	load_psf_file(fn[fcur]);
	
    /* set the audio format */
    wanted.freq = 44100;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = ABUFSIZ;
    wanted.callback = fill_audio_alt;
    wanted.userdata = 0;

    /* open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 )
    {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return 0;
    }
    
    SDL_PauseAudio(0);
    
    ispaused=0;
    chtrack=0;

	tick=0;
	
	while (run)
	{
		while ( SDL_PollEvent( &e ) )
		{
			switch ( e.type )
			{
			case SDL_QUIT:
				run = false;
				break;
			case SDL_KEYDOWN:
				
				switch ( e.key.keysym.sym )
				{
				/*case SDLK_q:*/
				case SDLK_ESCAPE:
					run = 0;
					break;
					
				case SDLK_SPACE:
					if (ispaused)
					{
						SDL_PauseAudio(0);
						ispaused=0;
					}
					else
					{
						SDL_PauseAudio(1);
						ispaused=1;
					}
					break;
				
				case SDLK_m:
					if (mono==1)
						mono=0;
					else
						mono=1;
					break;
				
				case SDLK_LEFT:
					if( fcur>0)
						fcur--;
					chtrack=1;
					break;
				
				case SDLK_RIGHT:
					if (fcur<(fcnt-1))
						fcur++;
					
					chtrack=1;
					
					break;
				
				case SDLK_0:
					if (ao_sample_limit[0]==0)
						ao_sample_limit[0]=1;
					else
						ao_sample_limit[0]=0;
					break;
				case SDLK_1:
					ao_sample_do[0] = ao_sample_do[0] ? 0 : 1;
					break;
				case SDLK_2:
					ao_sample_do[1] = ao_sample_do[1] ? 0 : 1;
					break;
				case SDLK_3:
					ao_sample_do[2] = ao_sample_do[2] ? 0 : 1;
					break;
				case SDLK_4:
					ao_sample_do[3] = ao_sample_do[3] ? 0 : 1;
					break;
				case SDLK_5:
					ao_sample_do[4] = ao_sample_do[4] ? 0 : 1;
					break;
				case SDLK_6:
					ao_sample_do[5] = ao_sample_do[5] ? 0 : 1;
					break;
				case SDLK_7:
					ao_sample_do[6] = ao_sample_do[6] ? 0 : 1;
					break;
				case SDLK_8:
					ao_sample_do[7] = ao_sample_do[7] ? 0 : 1;
					break;
				case SDLK_9:
					ao_sample_do[8] = ao_sample_do[8] ? 0 : 1;
					break;
				case SDLK_q:
					ao_sample_do[9] = ao_sample_do[9] ? 0 : 1;
					break;
				case SDLK_w:
					ao_sample_do[10] = ao_sample_do[10] ? 0 : 1;
					break;
				case SDLK_e:
					ao_sample_do[11] = ao_sample_do[11] ? 0 : 1;
					break;
				case SDLK_r:
					ao_sample_do[12] = ao_sample_do[12] ? 0 : 1;
					break;
				case SDLK_t:
					ao_sample_do[13] = ao_sample_do[13] ? 0 : 1;
					break;
				case SDLK_y:
					ao_sample_do[14] = ao_sample_do[14] ? 0 : 1;
					break;
				case SDLK_u:
					ao_sample_do[15] = ao_sample_do[15] ? 0 : 1;
					break;
				case SDLK_i:
					ao_sample_do[16] = ao_sample_do[16] ? 0 : 1;
					break;
				case SDLK_o:
					ao_sample_do[17] = ao_sample_do[17] ? 0 : 1;
					break;
				case SDLK_p:
					ao_sample_do[18] = ao_sample_do[18] ? 0 : 1;
					break;
				
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		
		
		SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, BG_COLOR));
		
		
		/*printf("sc update...\n");*/
		if (ispaused==0)
			dump_scope_buf();
		update_scope(screen);
		/*printf("ok\n");*/
		
		update_ao_chdisp(screen);
		
		update_ao_ch_flag_disp(screen,2);
		
		if (get_corlett_title() != 0)
		{
			
			
			render_text(
				screen,
				get_corlett_title(),
				&tcol_a,
				&tcol_b,
				font,
				10,8+0,
				1,SCREENWIDTH/2,tick);
			
			render_text(
				screen,
				get_corlett_artist(),
				&tcol_a,
				&tcol_b,
				font,
				10,8+12,
				1,SCREENWIDTH/2,tick);
				
			render_text(
				screen,
				get_corlett_game(),
				&tcol_a,
				&tcol_b,
				font,
				10,8+24,
				1,SCREENWIDTH/2,tick);
			
				
		}
		
		
		
		if (ispaused)
			render_text(
				screen, "- paused -", &tcol_a, &tcol_b,
				font, 10,SCREENHEIGHT - 16,1 ,SCREENWIDTH/2 ,tick);
		
		if (mono==1)
			render_text(
				screen, "mono", &tcol_a, &tcol_b,
				font, SCREENWIDTH-64, 8,  1,SCREENWIDTH/2,tick);
		else
			render_text(
				screen, "stereo", &tcol_a, &tcol_b,
				font, SCREENWIDTH-64, 8,  1,SCREENWIDTH/2,tick);
		
		
		
		
		if (chtrack)
		{
			SDL_PauseAudio(1);
			close_psf_file();
			printf("audio loading...\n");
			
			ao_sample_idx_clear();
			
			/* HERE */
			printf("file: %s\n",fn[fcur]);
			
			load_psf_file(fn[fcur]);
			printf("audio unpause...\n");
			
			reset_u_buf();
			
			if (ispaused==0)
				SDL_PauseAudio(0);
				
			printf("ok\n");
			
			clear_scope();
			update_scope(screen);
			
			chtrack=0;
			tick=0;
			newtrackcnt=20;
		}
		
		
		sprintf(tmpstr,"(%d/%d)",fcur+1,fcnt);
		
		render_text(
			screen,
			tmpstr,
			&tcol_a,
			&tcol_b,
			font,
			10,8+36,
			1,SCREENWIDTH/2,tick);
		
		
		set_channel_enable(channel_enable);
		
		SDL_UpdateWindowSurface(win);
		
		
        SDL_Delay(1000/60);
        
        if (newtrackcnt<0)
			tick++;
        
        newtrackcnt--;
	
	}
	
	SDL_CloseAudio();
	close_psf_file();
	
	
}
