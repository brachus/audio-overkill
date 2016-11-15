
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "audstrings.h"

int strcmp_nocase (const char * a, const char * b, int len)
{
    if (! a)
        return b ? -1 : 0;
    if (! b)
        return 1;

    return len < 0 ? g_ascii_strcasecmp (a, b) : g_ascii_strncasecmp (a, b, len);
}

