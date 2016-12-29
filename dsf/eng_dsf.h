
#include "../ao.h"
#include "../corlett_newer.h"

int dsf_lib(int libnum, uint8 *lib, uint64 size, corlett_t *c);
int32 dsf_start(uint8 *buffer, uint32 length);
int32 dsf_sample(int16_t *l, int16_t *r);
int32 dsf_frame(void);
int32 dsf_stop(void);
int32 dsf_command(int32 command, int32 parameter);
//int32 dsf_fill_info(ao_display_info *info);
