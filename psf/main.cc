
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "eng_protos.h"

#include "ao.h"

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


static int fok=AO_FAIL;
static int samples=44100 / 30;

static int16_t scope[SCOPEWIDTH];
static int16_t scope_buf[1024];
static int scope_bufsiz = -1;
static int scope_clear = 1;
static int scope_dump_max = 1024;

static float scope_view = 0.5;

static int mono = 0;





static int u_size = 3000;
static Uint8  *u_buf = ( Uint8 *) malloc(sizeof ( Uint8 ) * 3000);


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


void init()
{
	if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
		exit( EXIT_FAILURE );
	atexit( SDL_Quit );
	
	
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
	/*int mono=1;*/
	
	if (mono==0)
	{
		for (i=0;i<size;i++)
			u_buf[i] = buf[i];
	}
	else
	{
		for (i=0;i<size;i++) /* assuming two channels */
		{
			if (i%4==0)
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
		ctmp=str_prepend(ctmp,"file://");
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
	
	SDL_PixelFormat *fmt;
	int bpp, pitch, i, x, y;
	uint32_t ucol, ucol1, ucol2;
	uint8_t *pixel;
	
	float fy;
	
	
	
	fmt=in->format;
	
	bpp = fmt->BytesPerPixel;
	pitch = in->pitch;
	
	if (bpp==4)
	{
		SDL_FillRect(in, 0, SDL_MapRGB(fmt, BG_COLOR));
		
		ucol = SDL_MapRGB(fmt, SCOPE_COLOR);
		
		
		for (i=0;i<SCOPEWIDTH;i++)
		{
			x=i;
			fy=(float) scope[i] / (0xffff/2);
			
			y = (int) (fy * (SCREENHEIGHT/2));
			y += (SCREENHEIGHT/2);
			y = y<0? 0:y;
			y = y>(SCREENHEIGHT-1)?(SCREENHEIGHT-1):y;
			
			
			pixel = (uint8_t*) in->pixels;
			pixel += (y * pitch) + (x * bpp);
			*((uint32_t*)pixel) = ucol;
			
			pixel = (uint8_t*) in->pixels;
			pixel += ((y+1) * pitch) + (x * bpp);
			*((uint32_t*)pixel) = ucol;
			
		}
	}
	
	
	SDL_LockSurface( in );
	
	
	
	SDL_UnlockSurface( in );
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
	
	int maxtks, tkframe, shiftx, toobig=0;
	SDL_Rect tclip,tr;
	tclip.x=0;
	tclip.y=0;
	tclip.h=256;
	
	
	
	
	tr.w=SCREENWIDTH;
	tr.h=SCREENHEIGHT;
	
	static SDL_Surface* text;
	
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
	}
	
	
	SDL_FreeSurface(text);
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
	
	load_psf_file(fn[fcur]);
	
    /* set the audio format */
    wanted.freq = 44100;
    wanted.format = AUDIO_S16;
    wanted.channels = 2;    /* 1 = mono, 2 = stereo */
    wanted.samples = samples;
    wanted.callback = fill_audio;
    wanted.userdata = 0;

    /* open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return(-1);
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
				case SDLK_q:
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
		
		
		if (get_corlett_title() != 0)
		{
			sprintf(tmpstr,"(%d/%d)",fcur+1,fcnt);
			
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
			render_text(
				screen,
				tmpstr,
				&tcol_a,
				&tcol_b,
				font,
				10,8+36,
				1,SCREENWIDTH/2,tick);
				
			
		}
		
		if (ispaused)
			render_text(
				screen, "- paused -", &tcol_a, &tcol_b,
				font, 10,SCREENHEIGHT - 16,  1,SCREENWIDTH/2,tick);
		
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
			
			/* HERE */
			printf("file: %s\n",fn[fcur]);
			
			load_psf_file(fn[fcur]);
			printf("audio unpause...\n");
			
			if (ispaused==0)
				SDL_PauseAudio(0);
				
			printf("ok\n");
			
			clear_scope();
			update_scope(screen);
			
			chtrack=0;
			tick=0;
			newtrackcnt=20;
		}
		
		
		SDL_UpdateWindowSurface(win);
		
		
        SDL_Delay(1000/60);
        
        if (newtrackcnt<0)
			tick++;
        
        newtrackcnt--;
	
	}
	
	SDL_CloseAudio();
	close_psf_file();
	
	
}
