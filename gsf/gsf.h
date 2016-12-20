
extern void DisplayError (char *, ...);

extern int IsTagPresent (char *);
extern int IsValidGSF (char *);
extern void setupSound(void);
extern int GSFRun(char *);
extern void GSFClose(void) ;
extern int EmulationLoop(void);
extern int emu_loop(void);

extern void (*tmp_update )(const void *, int);
