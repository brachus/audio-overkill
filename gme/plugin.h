

#define SAMPLERATE 2048


#ifdef __cplusplus

extern "C"
{
	int gme_execute ( void (*update)(const void *, int ));
	
	int gme_open ( char * fn);
	
	void gme_close ( void );
};

#endif

int gme_execute ( void (*update)(const void *, int ));

int gme_open ( char * fn);

void gme_close ( void );
