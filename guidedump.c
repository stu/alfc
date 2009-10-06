#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "dlist.h"
#include "lzss.h"

#include "guideheader.h"
#include "guideload.h"

static void DumpGuide(uIM_GuideHeader *hdr)
{
	DLElement *e;
	uIM_Node *node;

	printf("%s\n", hdr->title);
	printf("%s\n", hdr->author);
	printf("%s\n", hdr->revision);
	printf("----\n");

	e = dlist_head(hdr->lstNodes);

	while (e != NULL)
	{
		int k;
		char *p;

		node = dlist_data(e);
		e = dlist_next(e);

		printf("%%");

		for (k = 0; k < node->node_count; k++)
		{
			if (k > 0)
				printf("%%");

			printf("%s", node->nodes[k]);
		}
		printf("%%\n");

		p = (char*) node->data;

		while (*p != 0)
		{
			if (*p == 0x0A)
				printf("\n");
			else
				printf("%c", *p);

			p++;
		}
	}

	fflush(stdout);
}

int main(int argc, char *argv[])
{
	uIM_GuideHeader *hdr;

	if (argc != 2)
	{
		printf("GuideDump v0.1\n");
		printf("syntax: gdump inputfile\n");
		exit(0);
	}

	hdr = LoadGuide(argv[1]);
	if (hdr != NULL)
		DumpGuide(hdr);

	return 0;
}
