
#include "conf.h"

#define CONF_FILE "./test.conf"



struct cfg_entry *new_cfg_entry()
{
	int i;
	struct cfg_entry *n = (struct cfg_entry *) malloc(sizeof(struct cfg_entry));
	
	if (n==0)
		return 0;
	
	n->name = 0;
	n->len = 0;
	
	for (i=0;i<4;i++)
	{
		n->dat[i].type = V_NONE;
		n->dat[i].val_str = 0;
		n->dat[i].val_int = 0;
		n->dat[i].val_float = 0.0;
	}
	
	n->next = 0;
	
	return n;
};

int append_cfg_entry(struct cfg_entry *o, struct cfg_entry *addme)
{
	struct cfg_entry *tmp1=0, *tmp;
	
	tmp1 = o;
	
	while (tmp1 != 0)
	{
		tmp = tmp1->next;
		
		if (tmp==0)
			break;
			
		tmp1=tmp;
	}
	
	if (tmp1==0)
		return 0;
	
	tmp1->next = addme;
	
	if (tmp1->next!=0)
		return 1;
	
	return 0;
}

void free_cfg_entries(struct cfg_entry *o)
{
	struct cfg_entry *tmp1, *tmp;
	
	if (!o)
		return;
		
	tmp1=o;
	
	while (tmp1)
	{
		tmp = tmp1->next;
		free(tmp1);
		tmp1=tmp;
		
	}
	
}

struct cfg_entry * read_entry(char *s)
{
	int i=0,tmp;
	int ints[4];
	char str[64], str1[64];
	
	struct cfg_entry *n = 0;
	
	if (!s)
		return 0;
		
	if (s[0]=='\0')
		return 0;
	
	
	for (i=0;i<4;i++)
		ints[i] = -1;
	
	/* rgb value ? */
	tmp = sscanf(s," %[^= \t] = %d , %d , %d ", str, &ints[0], &ints[1], &ints[2]);
	if (tmp==4)
	{
		n = new_cfg_entry();
		
		n->name = (char*) malloc(strlen(str) + 1);
		strcpy(n->name, str);
		
		n->len = 3;
		
		n->dat[0].type = V_INT;
		n->dat[0].val_int = ints[0];
		
		n->dat[1].type = V_INT;
		n->dat[1].val_int = ints[1];
		
		n->dat[2].type = V_INT;
		n->dat[2].val_int = ints[2];
		
		return n;
	}
	
	/* single int ? */
	tmp = sscanf(s," %[^= \t] = %d ", str, &ints[0]);
	if (tmp==2)
	{
		n = new_cfg_entry();
		
		n->name = (char*) malloc(strlen(str) + 1);
		strcpy(n->name, str);
		
		n->len = 1;
		
		n->dat[0].type = V_INT;
		n->dat[0].val_int = ints[0];
		
		return n;
	}
	
	/* string value ? */
	tmp = sscanf(s," %[^= \t] = %[^=,\t]", str, str1);
	if (tmp==2)
	{
		n = new_cfg_entry();
		
		n->name = (char*) malloc(strlen(str) + 1);
		strcpy(n->name, str);
		
		n->len = 1;
		
		n->dat[0].type = V_STR;
		n->dat[0].val_str = (char*) malloc(strlen(str1) + 1);
		strcpy(n->dat[0].val_str, str1);
		
		return n;
	}
	
	return n;
};


struct cfg_entry * read_conf(char *fn)
{
	int j;
	char chtmp[256], md, ch;
	FILE *fp;
	struct cfg_entry *dat = 0;
	
	
	if (!(fp = fopen(fn, "r")))
		return 0;
	
	
	md='o';
	j=0;
	
	while ((ch=getc(fp))!=EOF)
	{
		if (md=='o')
		{
			if (ch=='\n' || ch=='#')
			{
				
				if (dat==0)
					dat = read_entry(chtmp);
				else
					append_cfg_entry(dat, read_entry(chtmp) );
					
				j=0;chtmp[j]= '\0';
				
				
				if (ch=='#')
					md='c';
					
			}
			else
			{
				if (j+1 < 256)
				{
					chtmp[j] = ch;
					j++;
					chtmp[j] = '\0';
				}
				
			}
		}
		
		else if (md=='c')
			if (ch=='\n')
				md='o';
	}
	
	fclose(fp);
	
	return dat;
};

void print_cfg_entries(struct cfg_entry *o)
{
	int i;
	struct cfg_entry *tmp;
	
	tmp = o;
	
	while (tmp!=0)
	{
		printf("%s = ",tmp->name);
		
		for (i=0;i<tmp->len;i++)
		{
			if (tmp->dat[i].type == V_INT)
				printf("%d", tmp->dat[i].val_int);
			if (tmp->dat[i].type == V_FLOAT)
				printf("%f", tmp->dat[i].val_float);
			if (tmp->dat[i].type == V_STR)
				printf("%s", tmp->dat[i].val_str);
			
			
			if (i!=tmp->len-1)
				printf(", ");
		}
		
		
		printf("\n");
		
		
		tmp = tmp->next;
	}
}
