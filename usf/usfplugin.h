

#ifndef _USF_PLUGIN_H
#define _USF_PLUGIN_H


extern int16_t samplebuf[16384];
extern int SampleRate;


void add_buffer(unsigned char *buf, unsigned int length);
void ai_len_changed();
unsigned ai_read_length();
void ai_dacrate_changed(unsigned int value);

#endif
