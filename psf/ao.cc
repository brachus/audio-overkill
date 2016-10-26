
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#include "corlett.h"
#include "eng_protos.h"

#include <libaudcore/index.h>
#include <libaudcore/audstrings.h>
#include <libaudcore/vfs.h>

static String dirpath;

/* ao_get_lib: called to load secondary files */
Index<char> ao_get_lib(char *libdir, char *filename)
{
	if (!libdir)
		dirpath = (String) "file://./";
	else
		dirpath = (String) libdir;
	
	printf("lib str %s\n", libdir);
	
    VFSFile file(filename_build({dirpath, filename}), "r");
    return file ? file.read_all() : Index<char>();
}
