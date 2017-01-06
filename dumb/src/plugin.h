

#include "../../ao.h"

int pduh_execute ( void (*update)(const void *, int ));
int pduh_open (char * fn);
void pduh_close ( void );


struct DUH
{
	long length;

	int n_tags;
	char *(*tag)[2];

	int n_signals;
	void **signal;
};
