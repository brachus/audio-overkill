

#include "usf.h"

#include <glib.h>

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

extern "C"
{
	#include "usfplugin.h"
}



void (*tmp_usf_update)(const void *, int );


int usf_execute ( void (*update)(const void *, int ))
{
	
	if (!cpu_running)
	{
		update(0,0);
		return 0;
	}
	
	tmp_usf_update = update;
	
	StartEmulationFromSave(savestatespace); /* savestatespace from memory.h */
	/* no longer calls StartInterpreterCPU.*/
	
	StartInterpreterCPU();
	
	
}

int usf_open (char * fn)
{
	
	struct filebuf fb = filebuf_init();
	
	if (!filebuf_load(fn, fb))
	{
		filebuf_free(fb);
		return 0;
	}
	
	usf_init();
	
	usf_play(fn, fb); /* no longer calls StartEmulationFromSave. */
}

void usf_close ( void )
{
	if (cpu_running)
	{
		cpu_running = 0;
		CloseCpu();
	}
	

    usf_playing = false;
    is_paused = 0;
}


void add_buffer(unsigned char *buf, unsigned int length){
    int32_t i = 0, out = 0;
    double vol = 1.0;

    if (!cpu_running)
		return;

    if (is_seeking) {
		play_time += (((double) (length >> 2) / (double) SampleRate) * 1000.0);
		if (play_time > (double) seek_time)
			is_seeking = 0;
		
		return;
    }

    if (play_time > track_time)
    {
		vol = 1.0f -
			(((double) play_time -
			  (double) track_time) / (double) fade_time);
    }
	
	
	/* CALL UPDATE().  LENGTH SHOULD BE IN BYTES, DATA SHOULD BE SIGNED 16 INT 2 CHANNEL. */
    for (out = i = 0; i < (length >> 1); i += 2)
    {
		samplebuf[out++] =
			(int16_t) (vol * (double) ((int16_t *) buf)[i + 1]);
		samplebuf[out++] = (int16_t) (vol * (double) ((int16_t *) buf)[i]);
    }

    play_time += (((double) (length >> 2) / (double) SampleRate) * 1000.0);

    usf_playing = play_time < (track_time + fade_time);

	
    tmp_usf_update((uint8_t *) samplebuf, length * sizeof (int16_t));

    /*if (play_time > (track_time + fade_time)) {
	cpu_running = 0;
    }*/
    
    /* play forever for now. */
}

void ai_len_changed(){
    int32_t length = 0;
    uint32_t address = (AI_DRAM_ADDR_REG & 0x00FFFFF8);

    length = AI_LEN_REG & 0x3FFF8;

    while (is_paused && !is_seeking && cpu_running)
	g_usleep(10000);


    add_buffer(RDRAM + address, length);

    if (length && !(AI_STATUS_REG & 0x80000000)) {
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

    if (enableFIFOfull) {
	if (AI_STATUS_REG & 0x40000000)
	    AI_STATUS_REG |= 0x80000000;
    }

    AI_STATUS_REG |= 0x40000000;
    

}

unsigned ai_read_length(){
    AI_LEN_REG = 0;
    return AI_LEN_REG;
}

void ai_dacrate_changed(unsigned int value){
    AI_DACRATE_REG = value;
    SampleRate = 48681812 / (AI_DACRATE_REG + 1);
}
