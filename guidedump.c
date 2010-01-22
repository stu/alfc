#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>

#include "dlist.h"
#include "guideheader.h"
#include "guideload.h"


#ifdef BUILD_WIN32
char* ALFC_getenv(const char *s)
{
	char *p;

	if(strcmp(s, "HOME") == 0)
	{
		p = getenv("HOME");
		if(p != NULL)
			return p;

		p = getenv("USERPROFILE");
		if(p != NULL)
			return p;

		return NULL;
	}
	else
		return getenv(s);
}
#else
char* ALFC_getenv(const char *s)
{
	return getenv(s);
}
#endif

/* Convert environment variables to actual strings */
char* ConvertDirectoryName(const char *x)
{
	char *env;

	char *a;
	char *p;
	char *q;
	char *z;

	if (x == NULL)
		return strdup("");

	a = strdup(x);
	p = malloc(1024);
	assert(p != NULL);

	p[0] = 0;
	q = a;

	if (q[0] == '~')
	{
		strcpy(p, "$HOME");
		strcat(p, q + 1);

		free(a);
		a = strdup(p);
	}

	p[0] = 0;

	while (*q != 0)
	{
		char *idx;

		env = NULL;
		idx = strchr(q, '$');

		if (idx != NULL)
		{
			char *z;
			char c;

			*idx = 0;

			strcat(p, q);
			q = idx + 1;

			z = strchr(q, '/');
			if (z == NULL)
				z = strchr(q, 0);

			c = *z;
			*z = 0;

			env = ALFC_getenv(q);
			*z = c;

			if (env != NULL)
			{
				if (p[0] != 0 && p[strlen(p) - 1] != '/')
					strcat(p, "/");

				if (p[0] != 0 && *env == '/')
					env += 1;

				strcat(p, env);
				q = z;
			}
			else
			{
				strcat(p, "$");
				q = idx + 1;
			}
		}
		else
		{
			strcat(p, q);
			q = strchr(q, 0);
		}
	}

	for (q = p, z = p; *q != 0;)
	{
		switch (*q)
		{
			case '\\':
				*z = '/';
				z++;
				q++;
				break;

			case '"':
				q++;
				break;

			default:
				*z = *q;
				z++;
				q++;
				break;
		}
	}

	*z = 0;
	free(a);

	return realloc(p, 16 + strlen(p));
}


static void DumpGuide(uHelpFile *hlp)
{
	DLElement *e;

	printf("\\title{%s}\n", hlp->title);
	printf("\\author{%s}\n", hlp->author);
	printf("\\version{%s}\n", hlp->revision);

	e = dlist_head(hlp->lstSections);

	while (e != NULL)
	{
		DLElement *el;

		uHelpSection *sect;

		sect = dlist_data(e);
		e = dlist_next(e);

		printf("\n\\section{%s}\n", sect->name);

		el = dlist_head(sect->lstLines);
		while (el != NULL)
		{
			printf("%s\n", (char*) dlist_data(el));
			el = dlist_next(el);
		}
	}

	printf("\n");

	fflush(stdout);
}

int main(int argc, char *argv[])
{
	uHelpFile *hlp;

	if (argc != 2)
	{
		printf("GuideDump v0.1\n");
		printf("syntax: gdump inputfile\n");
		exit(0);
	}

	hlp = LoadHelpFile(argv[1]);
	if (hlp != NULL)
	{
		DumpGuide(hlp);
		FreeHelpFile(hlp);
	}

	return 0;
}
