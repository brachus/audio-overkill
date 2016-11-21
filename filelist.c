
#include "filelist.h"


struct flist_base *flist_init()
{
	struct flist_base *n;
	n=(struct flist_base*)malloc(sizeof(struct flist_base));
	
	n->len=0;
	n->dat=0;
	
	return n;
};

struct flist_item *flist_item_init(char *s)
{
	struct flist_item *n;
	
	n=(struct flist_item*)malloc(sizeof(struct flist_item));
	
	n->nm = (char*) malloc(strlen(s)+1);
	strcpy(n->nm, s);
	
	n->next = 0;
	
	return n;

};


void add_flist_item(struct flist_base *n, char *s)
{
	struct flist_item *tmp;
	
	if (s==0)
		return;
	
	if (strlen(s) == 0)
		return;
	
	tmp = n->dat;
	
	if (tmp==0)
	{
		n->dat = flist_item_init(s);
		n->len++;
		return;
	}
	
	while (tmp!=0)
	{
		if (tmp->next == 0)
		{
			tmp->next = flist_item_init(s);
			n->len++;
			break;
		}
		
		tmp = tmp->next;
	}
};


char * get_flist_idx(struct flist_base *n,int idx)
{
	struct flist_item *tmp;
	int i;
	
	tmp = n->dat;
	i=0;
	
	while (tmp!=0 && i < n->len)
	{
		
		if (idx == i)
			return tmp->nm;
			
		tmp = tmp->next;
		i++;
	}
	
	return 0;
};

