

#include "usf.h"

#include <glib.h>

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "../gsf/libresample-0.1.3/include/libresample.h" /* steal this for now. */

#include "usfplugin.h"

extern "C"
{
	#include "usfpluginout.h"
	
	#include "../ao.h"
}

int16_t samplebuf[16384];

struct filebuf *uopen_fb;

float samplebuf_44k[44100*2];
float samplebuf_44k_b[44100*2];
float samplebuf_44k_c[44100*2];
float samplebuf_44k_d[44100*2];
int16_t samplebuf_44k_e[44100*2];

void * handle[2];

int cpu_init=0;


void (*tmp_usf_update)(const void *, int );


int usf_execute ( void (*update)(const void *, int ))
{
	
	cpu_running=1;
	
	tmp_usf_update = update;
	if (!cpu_init)
	{
		StartEmulationFromSave(savestatespace); /* savestatespace from memory.h */
		cpu_init=1;
	}
		
	cpu_stopped = 0;
    cpu_running = 1;
    fake_seek_stopping = 0;
    
    if (CPU_Type == CPU_Interpreter)
		StartInterpreterCPU();
	else if (CPU_Type == CPU_Recompiler)
		StartRecompilerCPU();
	
	return 1;
}

int usf_open (char * fn)
{
	
	uopen_fb = filebuf_init();
	
	if (!filebuf_load(fn, uopen_fb))
	{
		filebuf_free(uopen_fb);
		uopen_fb = 0;
		return 0;
	}
	
	cpu_init = 0;
	
	usf_init();
	
	usf_play(fn, uopen_fb); /* no longer calls StartEmulationFromSave. */
	
	handle[0] = resample_open(1, 44100. / SampleRate, 44100. / SampleRate);
    handle[1] = resample_open(1, 44100. / SampleRate, 44100. / SampleRate);
    
    
}

void usf_close ( void )
{
	if (cpu_running)
		CloseCpu();
	
	cpu_running = 0;
	
	if (uopen_fb != 0)
		filebuf_free(uopen_fb);
	
	Release_Memory();
	
}


void add_buffer(unsigned char *buf, unsigned int length){
    int32_t i = 0, out = 0;
    
    double vol = 1.0;
    
    int out_final = 0;
    
    
    
    double rs_ratio = 44100. / SampleRate;
    
    int inused, re_out;
    
    
    
    //samplebuf_44k
    
    int playforever = 1;

    if (!cpu_running)
		return;

    if (is_seeking)
    {
		play_time += (((double) (length >> 2) / (double) SampleRate) * 1000.0);
		if (play_time > (double) seek_time)
			is_seeking = 0;
		
		return;
    }

    if (play_time > track_time && !playforever)
    {
		vol = 1.0f -
			(((double) play_time -
			  (double) track_time) / (double) fade_time);
    }
	
	
	
	/* CALL UPDATE().  LENGTH SHOULD BE IN BYTES, DATA SHOULD BE SIGNED 16 INT 2 CHANNEL. */
    for (out = i = 0; i < (length >> 1); i += 2)
    {
		/*samplebuf[out++] =
			(int16_t) (vol * (double) ((int16_t *) buf)[i + 1]);
		samplebuf[out++] = (int16_t) (vol * (double) ((int16_t *) buf)[i]);*/
		
		
		samplebuf_44k[out] = (float) (((int16_t *)buf)[i]) / 32768. ;
		samplebuf_44k_b[out++] = (float) (((int16_t *)buf)[i+1]) / 32768. ;
		
		
		out_final = out;
		
    }
    
    
    
    re_out=resample_process(
		handle[0],
		rs_ratio,
		samplebuf_44k,
		out_final,
		0, /*last flag*/
        &inused,
        samplebuf_44k_c,
        44100);
    //printf("orig:%d used:%d  reout:%d\n",out_final,inused,re_out);
    inused=0;
    re_out = resample_process(
		handle[1],
		rs_ratio,
		samplebuf_44k_b,
		out_final,
		0, /*lflag*/
        &inused,
        samplebuf_44k_d,
        44100);
    //printf("  orig:%d used:%d  reout:%d\n",out_final,inused,re_out);
    
    for (i=0;i<re_out;i++)
    {
		if (samplebuf_44k_c[i] > 0.98)samplebuf_44k_c[i] = .98;
		if (samplebuf_44k_c[i] < -0.98)samplebuf_44k_c[i] = -0.98;
		if (samplebuf_44k_d[i] > 0.98)samplebuf_44k_d[i] = .98;
		if (samplebuf_44k_d[i] < -0.98)samplebuf_44k_d[i] = -0.98;
		
		samplebuf_44k_e[i*2] = (int16_t) ((float) samplebuf_44k_c[i] * 32766.);
		samplebuf_44k_e[i*2 + 1] = (int16_t) ((float) samplebuf_44k_d[i] * 32766.);
	}
    /*
    
    printf("c\n");
    for (out = 0; (out+2) < 44100;out+=2)
    {
		samplebuf_44k[out] = (int16_t) interp_pop(0, 1./0.5);
		samplebuf_44k[out+1] = (int16_t) interp_pop(1, 1./0.5);
		
		out_final = out + 2;
	}*/

    play_time += (((double) (length >> 2) / (double) SampleRate) * 1000.0);

    //usf_playing = play_time < (track_time + fade_time);

	if (playforever || play_time < (track_time + fade_time))
		//tmp_usf_update((uint8_t *) samplebuf, length);
		tmp_usf_update((uint8_t *) samplebuf_44k_e, re_out*2*2);
	
	else
		tmp_usf_update(0, 0);
	
    
	cpu_running=0;
}

void ai_len_changed(){
    int32_t length = 0;
    uint32_t address = (AI_DRAM_ADDR_REG & 0x00FFFFF8);

    length = AI_LEN_REG & 0x3FFF8;

    //while (is_paused && !is_seeking && cpu_running)
	//g_usleep(10000);


    add_buffer(RDRAM + address, length);

    if (length && !(AI_STATUS_REG & 0x80000000))
    {
		const float VSyncTiming = 789000.0f;
		double BytesPerSecond = 48681812.0 / (AI_DACRATE_REG + 1) * 4;
		double CountsPerSecond =
			(double) ((((double) VSyncTiming) * (double) 60.0)) * 2.0;
		double CountsPerByte =
			(double) CountsPerSecond / (double) BytesPerSecond;
		unsigned int IntScheduled =
			(unsigned int) ((double) AI_LEN_REG * CountsPerByte);

		ChangeTimer(AiTimer, IntScheduled);
    }

    if (enableFIFOfull)
    {
		if (AI_STATUS_REG & 0x40000000)
			AI_STATUS_REG |= 0x80000000;
    }

    AI_STATUS_REG |= 0x40000000;
    

}

unsigned ai_read_length()
{
    AI_LEN_REG = 0;
    return AI_LEN_REG;
}

void ai_dacrate_changed(unsigned int value)
{
    AI_DACRATE_REG = value;
    SampleRate = 48681812 / (AI_DACRATE_REG + 1);
    
    handle[0] = resample_open(1, 44100. / SampleRate, 44100. / SampleRate);
    handle[1] = resample_open(1, 44100. / SampleRate, 44100. / SampleRate);
}
