
#include "conf.h"

#define CONF_FILE "./test.conf"


static char parse_sec[64] = "general";

struct cfg_entry *new_cfg_entry()
{
	int i;
	struct cfg_entry *n = (struct cfg_entry *) malloc(sizeof(struct cfg_entry));
	
	if (n==0)
		return 0;
	
	/*n->name = 0;*/
	n->len = 0;
	
	n->type = E_NONE;
	/*n->section = 0;*/
	
	for (i=0;i<4;i++)
	{
		n->dat[i].s = 0;
		n->dat[i].i = 0;
		n->dat[i].f = 0.0;
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
	int i;
	struct cfg_entry *tmp1, *tmp;
	
	if (!o)
		return;
		
	tmp1=o;
	
	while (tmp1)
	{
		tmp = tmp1->next;
		
		for (i=0;i<4;i++)
			if (tmp1->dat[i].s!=0)
				free(tmp1->dat[i].s);
			
		
		free(tmp1);
		tmp1=tmp;
		
	}
	
}


struct cfg_entry * read_entry(char *s)
{
	int i,j,tmp,len;
	
	int ints[4] = {0,0,0,0};
	float floats[4] = {0,0,0,0};
	char *strs[4] = {0,0,0,0};
	
	char types[4] = {0,0,0,0};
	
	int valcnt = 0;
	
	static char str[64], str1[512];
	char md,ch;
	
	
	enum
	{
		MOPEN,
		MSECTION,
		MKEY,
		MEQUAL,
		MVALUE,
		MCOMMA,
		MINT,
		MFLOAT,
		MSTR,
		MHEX,
		MBOOL
	};
	
	#define STR_RESET(x)  j=0;x[0]='\0';
	
	#define STR_ADD(x,max) if (j+1<max)\
				{\
					x[j]=ch;\
					j++;\
					x[j]='\0';\
				}\
	
	#define IF_CH_WSPC (ch==' ' || ch=='\t' || ch=='\n')
	
	
	struct cfg_entry *n = 0;
	
	if (!s)
		return 0;
		
	if (s[0]=='\0')
		return 0;
	
	md=MOPEN;
	
	len=strlen(s);
	i=0;
	
	STR_RESET(str);
	STR_RESET(str1);
	
	
	while (i<len && valcnt<4)
	{
		ch=s[i];
		
		if (ch=='#' || ch==';')
			break;
		
		
		if (md==MOPEN)
		{
			if (ch == '[')
			{
				STR_RESET(str);
				md=MSECTION;
			}
			else if (!IF_CH_WSPC)
			{
				STR_RESET(str);
				STR_ADD(str, 64);
				md=MKEY;
			}
		}
		else if (md==MSECTION)
		{
			if (ch != ']')
			{
				STR_ADD(str,64);
			}
			else
			{
				strcpy(parse_sec, str);
				
				STR_RESET(str);
				
				md=MOPEN;
			}
		}
		else if (md==MKEY)
		{
			if (!IF_CH_WSPC && ch!='=')
			{
				STR_ADD(str,64);
			}
			else if (IF_CH_WSPC)
			{
				md=MEQUAL;
			}
			else if (ch=='=')
			{
				md=MVALUE;
				
				STR_RESET(str1);
			}
		}
		else if (md==MEQUAL)
		{
			if (ch=='=')
			{
				md=MVALUE;
				
				STR_RESET(str1);
			}
		}
		else if (md==MVALUE)
		{
			if ((ch>='0' && ch<='9') || ch=='-')
			{
				md=MINT;
				
				STR_ADD(str1,512);
				
				if (i+1>=len)
				{
					ints[valcnt] = atoi(str1);
					types[valcnt] = MINT;
					valcnt++;
				}
				
			}
			else if (!IF_CH_WSPC && ch!=',')
			{
				md=MSTR;
				
				STR_ADD(str1,512);
				
				if (i+1>=len)
				{
					strs[valcnt] = (char*) malloc(strlen(str1)+1);
					strcpy(strs[valcnt], str1);
					types[valcnt] = MSTR;
					valcnt++;
					
					
					STR_RESET(str1);
				}
			}
		}
		else if (md==MINT)
		{
			if (ch=='.')
			{
				md=MFLOAT;
			}
			else if (ch=='x')
			{
				if (j==1 && str1[0] == '0')
					md=MHEX;
				else
					md=MSTR;
				
				STR_ADD(str1,512);
			}
			else if (ch>='0' && ch<='9')
			{
				STR_ADD(str1,512);
				
				if (i+1>=len)
				{
					ints[valcnt] = atoi(str1);
					types[valcnt] = MINT;
					valcnt++;
				}
			}
			else if (ch==',' || IF_CH_WSPC)
			{
				ints[valcnt] = atoi(str1);
				types[valcnt] = MINT;
				valcnt++;
				
				STR_RESET(str1);
				
				if (IF_CH_WSPC)
					md=MCOMMA;
				else
					md=MVALUE;
			}
			else
				md=MSTR;
		}
		else if (md==MSTR)
		{
			if (!IF_CH_WSPC && ch!=',' &&ch!='\0')
			{
				STR_ADD(str1,512);
				
				
				if (i+1>=len)
				{
					strs[valcnt] = (char*) malloc(strlen(str1)+1);
					strcpy(strs[valcnt], str1);
					types[valcnt] = MSTR;
					valcnt++;
					
					
					STR_RESET(str1);
				}
			}
			else
			{
				strs[valcnt] = (char*) malloc(strlen(str1)+1);
				strcpy(strs[valcnt], str1);
				types[valcnt] = MSTR;
				valcnt++;
				
				md=MVALUE;
				if (ch!=',')
					md=MCOMMA;
			}
		}
		else if (md==MHEX)
		{
			if (ch>='0' && ch<='9')
			{
				STR_ADD(str1,512);
				
				if (i+1>=len)
				{
					ints[valcnt] = strtol(str1, 0, 0);
					types[valcnt] = MINT;
					valcnt++;
				}
			}
			else if (IF_CH_WSPC || ch == ',')
			{
				ints[valcnt] = strtol(str1, 0, 0);
				types[valcnt] = MINT;
				valcnt++;
				
				STR_RESET(str1);
				
				if (IF_CH_WSPC)
					md=MCOMMA;
				else
					md=MVALUE;
			}
			else
				md=MSTR;
		}
		else if (md==MFLOAT)
		{
			
			if (ch>='0' && ch<='9')
			{
				STR_ADD(str1,512);
				if (i+1>=len)
				{
					floats[valcnt] = atof(str1);
					types[valcnt] = MFLOAT;
					valcnt++;
				}
			}
			else if (IF_CH_WSPC || ch == ',')
			{
				floats[valcnt] = atof(str1);
				types[valcnt] = MFLOAT;
				valcnt++;
				
				STR_RESET(str1);
				
				if (IF_CH_WSPC)
					md=MCOMMA;
				else
					md=MVALUE;
			}
			else
				md=MSTR;
		}
		else if (md==MCOMMA)
		{
			if (ch==',')
				md=MVALUE;
		}
		i++;
	}
	
	if (valcnt!=0 && types[0] != MFLOAT)
	{
		n = new_cfg_entry();
		
		strcpy(n->name, str);
		
		strcpy(n->section, parse_sec);
		
		n->len = valcnt;
		
		if (types[0] == MSTR)
		{
			if (strcmp(strs[0], "True")==0 || strcmp(strs[0], "true")==0)
			{
				n->type = E_BOOL;
				n->dat[0].i = 1;
			}
			else if (strcmp(strs[0], "False")==0 || strcmp(strs[0], "false")==0)
			{
				n->type = E_BOOL;
				n->dat[0].i = 0;
			}
			else
			{
				n->type = E_STR;
				n->dat[0].s = (char*) malloc(strlen(strs[0]) + 1);
				strcpy(n->dat[0].s, strs[0]);
			}
			
			for (i=0;i<4;i++)
				free(strs[i]);
		}
		else if (types[0]==MINT)
		{
			
			if (types[1]==MINT && types[2]==MINT)
			{
				n->type = E_RGB;
				
				n->dat[0].i = ints[0];
				n->dat[1].i = ints[1];
				n->dat[2].i = ints[2];
			}
			else
			{
				n->type = E_INT;
				
				n->dat[0].i = ints[0];
			}
			
		}
		else if (types[0]==MFLOAT)
		{
			n->type = E_INT;
			n->dat[0].f = floats[0];
		}
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
			if (ch=='\n' || ch=='#' || ch==';')
			{
				
				if (dat==0)
					dat = read_entry(chtmp);
				else
					append_cfg_entry(dat, read_entry(chtmp) );
					
				j=0;
				chtmp[j]= '\0';
				
				
				if (ch=='#' || ch==';')
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
		printf("%s.%s = ",tmp->section, tmp->name);
		
		switch (tmp->type)
		{
		case E_BOOL:
			if (tmp->dat[0].i)
				printf("true");
			else
				printf("false");
			break;
		case E_INT:
			printf("%d", tmp->dat[0].i);
			break;
		case E_STR:
			printf("%s", tmp->dat[0].s);
			break;
		case E_RGB:
			printf("%d,%d,%d", tmp->dat[0].i,tmp->dat[1].i,tmp->dat[2].i);
			break;
		default:
			printf("?");
			break;
		}
		
		
		
		printf("\n");
		
		
		tmp = tmp->next;
	}
}

