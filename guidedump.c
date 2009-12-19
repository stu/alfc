#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "dlist.h"
#include "guideheader.h"
#include "guideload.h"

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
