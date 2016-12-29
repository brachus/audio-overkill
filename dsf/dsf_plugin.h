
#ifndef _DSF_PLUGIN_H_
#define _DSF_PLUGIN_H_

#define DSF_SAMPLERATE (44100/60)

#include <stdio.h>
#include <stdlib.h>

#include "eng_dsf.h"

#include "../ao.h"

int load_dsf_file(char *fn);
void close_dsf_file();
int dsf_execute (void (*update)(const void *, int ));


#endif
