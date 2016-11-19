
#include <string.h>
#include <stdlib.h>


struct flist_base
{
	int len;
	struct flist_item *dat;
};

struct flist_item
{
	char *nm;
	struct flist_item *next;
};

struct flist_base *flist_init();
struct flist_item *flist_item_init(char *s);
void add_flist_item(struct flist_base *n, char *s);
char * get_flist_idx(struct flist_base *n,int idx);
