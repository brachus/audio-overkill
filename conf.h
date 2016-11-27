
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __AO_CONF_H
#define __AO_CONF_H

enum
{
	V_NONE,
	V_INT,
	V_FLOAT,
	V_STR
};

enum
{
	E_NONE,
	E_INT,
	E_BOOL,
	E_STR,
	E_RGB,
	E_FLOAT
};

struct cvar
{
	int i;
	char *s;
	float f;
};

struct cfg_entry
{
	char name[64];
	struct cvar dat[4];
	int len;
	
	int type;
	char section[64];
	
	struct cfg_entry *next;
};


struct cfg_entry *new_cfg_entry();
int append_cfg_entry(struct cfg_entry *o, struct cfg_entry *addme);
void free_cfg_entries(struct cfg_entry *o);
struct cfg_entry * read_entry(const char *s);
struct cfg_entry * read_conf(const char *fn);
void print_cfg_entries(struct cfg_entry *o);


#endif

