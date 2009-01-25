#include "headers.h"

/*
TODO : duplicate items/groups
*/

#ifdef PATH_MAX
#define MAX_EXECNAME_SIZE	PATH_MAX+2
#else
#define MAX_EXECNAME_SIZE  1024
#endif


#define MAX_INILINE_SIZE	1024
#define MAX_INIINDEX_SIZE	128
#define MAKE_NODE	(INIFILE *)calloc(1, sizeof(INIFILE));

struct udtItem
{
	char *name;
	char *value;
};

struct udtGroup
{
	char *name;
	DList *lstItems;
};

static void FreeINIGroup(void *data);
static void INI_parse(FILE *fp, INIFILE *f);
static void FreeINIItem(void *data);

INIFILE* INI_EmptyINF(void)
{
	INIFILE *f;

	f = malloc(sizeof(INIFILE));
	f->lstGroups = malloc(sizeof(DList));
	dlist_init(f->lstGroups, &FreeINIGroup);

	return f;
}

int INI_save(char *fname, INIFILE *f)
{
	FILE 	*fp;
	int 	errl=1;

	DLElement *egroup;
	DLElement *eitem;

	struct udtGroup *g;
	struct udtItem *i;

	if((fp=fopen(fname, "wt"))!=NULL)
	{
		egroup = dlist_head(f->lstGroups);

		while(egroup != NULL)
		{
			g = dlist_data(egroup);

			fprintf(fp, "%s\n", g->name);

			eitem = dlist_head(g->lstItems);
			while(eitem != NULL)
			{
				i = dlist_data(eitem);
				fprintf(fp, "%s = %s\n", i->name, i->value);

				eitem = dlist_next(eitem);

			}

			fprintf(fp, "\n");

			egroup = dlist_next(egroup);
		}

		fclose(fp);
		errl=0;
	}

	return errl;
}

void INI_UpdateItem(INIFILE *f, char *group, char *key, char *val)
{
	DLElement *egroup;
	struct udtGroup *g;
	struct udtItem *i;
	char *gname;

	gname = malloc(4 + strlen(group));

	strcpy(gname, "[");
	strcat(gname, group);
	strcat(gname, "]");

	egroup = dlist_head(f->lstGroups);

	while(egroup != NULL)
	{
		g = dlist_data(egroup);

		if(stricmp(gname, g->name) == 0)
		{
			DLElement *eitem;


			eitem = dlist_head(g->lstItems);
			while(eitem != NULL)
			{
				i = dlist_data(eitem);

				if(stricmp(key, i->name) == 0)
				{
					free(gname);

					// release old value and update with new
					free(i->value);
					i->value = strdup(val);
					return;
				}

				eitem = dlist_next(eitem);
			}

			// does not exist, so add new item to existing group
			i = malloc(sizeof(struct udtItem));
			i->name = strdup(key);
			i->value = strdup(val);
			dlist_ins(g->lstItems, i);

			free(gname);
			return;
		}

		egroup = dlist_next(egroup);
	}

	// does not exist in any groups...
	g = malloc(sizeof(struct udtGroup));
	g->name = strdup(gname);
	g->lstItems = malloc(sizeof(DList));
	dlist_init(g->lstItems, &FreeINIItem);
	dlist_ins(f->lstGroups, g);

	i = malloc(sizeof(struct udtItem));
	i->name = strdup(key);
	i->value = strdup(val);
	dlist_ins(g->lstItems, i);

	free(gname);
}

static void FreeINIItem(void *data)
{
	struct udtItem *x = data;

	free(x->name);
	free(x->value);
	free(x);
}

static void FreeINIGroup(void *data)
{
	struct udtGroup *g = data;

	dlist_destroy(g->lstItems);
	free(g->lstItems);

	free(g->name);
	free(g);
}

void INI_unload(INIFILE *f)
{
	dlist_destroy(f->lstGroups);
	free(f->lstGroups);
	free(f);
}

INIFILE *INI_load(char *fname)
{
	INIFILE *f;
	FILE *fp;

	fp = fopen(fname, "rt");
	if( fp == NULL )
		return NULL;

	f = malloc(sizeof(INIFILE));

	f->lstGroups = malloc(sizeof(DList));
	dlist_init(f->lstGroups, &FreeINIGroup);

	INI_parse(fp, f);
	fclose(fp);

	return f;
}

static char* QuotedName(char *q, int end)
{
	int quote = 0;
	int flag = 0;

	while( *q != 0 && flag == 0 )
	{
		if(*q == end && flag == 0)
		{
			//*q = 0;
			flag = 1;
			return q;
		}

		switch(*q)
		{
			case '#':
				if(quote == 0)
				{
					*q = 0;
					return q;
				}
				else
					q++;

				break;

			case '\"':
				quote = !quote;
				q++;
				break;

			case '\\':
				q += 2;
				break;

			default:
				q += 1;
				break;
		}
	}

	return q;
}

static void INI_parse(FILE *fp, INIFILE *f)
{
	char	line[MAX_INILINE_SIZE];
	char	*z, *x;

	char	*sym;
	char	*val;

	struct udtGroup *current_group;

	sym = NULL;
	val = NULL;

	current_group = NULL;

	if( fp != NULL)
	{
		do
		{
			*line = 0x0;
			z = fgets(line, MAX_INILINE_SIZE, fp);

			if( *line != 0x0)
			{
				x = strchr(line, 0x0A);		// strip cr/lf
				if(x != NULL)
					*x = 0x0;

				x=strchr(line, 0x0D);	// included just in case ^_^
				if(x != NULL)
					*x = 0x0;

				if(*line != 0)
				{
					z = line;

					// skip over whitespace.
					while( *z != 0 && isspace(*z) != 0)
						z++;

					if(*z == 0)
						break;

					sym = z;
					val = QuotedName(sym, '=');
					if(*val !=0 && *val=='=')
					{
						*val = 0;
						val++;
					}
					QuotedName(val, 0);

					z = val;
					while(z != NULL && *z != 0 && isspace(*z) != 0)
						z++;
					val = z;

					if(*sym != 0)
					{
						z = strchr(sym, 0x0);
						z--;

						while(z != sym && isspace(*z) != 0)
						{
							*z = 0;
							z--;
						}

						if(*val != 0)
						{
							z = strchr(val, 0x0);
							z--;

							while(z != val && isspace(*z) != 0)
							{
								*z = 0;
								z--;
							}
						}
					}

					if(*sym == '[' && sym[ strlen(sym) - 1 ] == ']')
					{
						current_group = malloc(sizeof(struct udtGroup));
						current_group->name = strdup(sym);
						current_group->lstItems = malloc(sizeof(DList));
						dlist_init(current_group->lstItems, &FreeINIItem);

						dlist_ins(f->lstGroups, current_group);
					}
					else
					{
						/* skip items outside a group */
						if(current_group != NULL && strlen(sym) > 0 )
						{
							struct udtItem *i;

							i = malloc(sizeof(struct udtItem));
							i->name = strdup(sym);
							i->value = strdup(val);

							dlist_ins(current_group->lstItems, i);
						}
					}
				}

			}

		}while(!feof(fp));
	}
}

char* INI_get(INIFILE *ini, char *group, char *item)
{
	DLElement *egroup;
	struct udtGroup *g;
	char *gname;

	gname = malloc(4 + strlen(group));

	strcpy(gname, "[");
	strcat(gname, group);
	strcat(gname, "]");

	egroup = dlist_head(ini->lstGroups);

	while(egroup != NULL)
	{
		g = dlist_data(egroup);

		if(stricmp(gname, g->name) == 0)
		{
			DLElement *eitem;
			struct udtItem *i;

			eitem = dlist_head(g->lstItems);
			while(eitem != NULL)
			{
				i = dlist_data(eitem);

				if(stricmp(item, i->name) == 0)
				{
					free(gname);
					return i->value;
				}

				eitem = dlist_next(eitem);
			}

			/* no matching item in group */
			free(gname);
			return NULL;
		}

		egroup = dlist_next(egroup);
	}

	free(gname);
	return NULL;
}


