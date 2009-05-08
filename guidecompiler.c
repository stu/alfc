#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "dlist.h"
#include "lzss.h"

// hack to skip this on my mingw system
#ifndef __MINGW_H
#include <bzlib.h>
#endif

#include <zlib.h>

#include "guideheader.h"

char *in_file;
char *out_file;

uint32_t inlen;
uint8_t *inbuff;

uint32_t in_total;
uint32_t out_total;

int comp_type;

static uIM_GuideHeader* AllocGuide(void)
{
	uIM_GuideHeader *x;

	x = calloc(1, sizeof(uIM_GuideHeader));

	x->lstNodes = malloc(sizeof(DList));
	dlist_init(x->lstNodes, NULL);

	return x;
}

static uIM_Node* AllocNode(void)
{
	uIM_Node *x = calloc(1, sizeof(uIM_Node));

	return x;
}

static int ReadInputFile(char *fn)
{
	FILE *fp;

	fp = fopen(fn, "rb");
	if (fp != NULL)
	{
		uint32_t rc;

		fseek(fp, 0x0L, SEEK_END);
		inlen = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);

		inbuff = calloc(1, 16 + inlen);
		assert(inbuff != NULL);

		rc = fread(inbuff, 1, inlen, fp);
		fclose(fp);

		assert(rc == inlen);

		return 0;
	}

	return 1;
}

static int IsCRLF(uint8_t c)
{
	if (c == 0x0A || c == 0x0D)
		return 1;
	else
		return 0;
}

static char* SkipLines(char *q, int count)
{
	int i;

	i = 0;
	while (i < count)
	{
		char *p;

		p = q;
		while (IsCRLF(*q) == 0 && q - p < inlen)
		{
			q++;
		}

		while (IsCRLF(*q) != 0 && q - p < inlen)
		{
			q++;
		}

		i += 1;
	}

	if (i == count)
		return q;

	return NULL;
}

static int ReadTitle(uIM_GuideHeader *x)
{
	char *p;
	char *q;

	q = SkipLines((char*) inbuff, 0);
	p = q;
	while (IsCRLF(*q) == 0 && q - p < inlen)
	{
		q++;
	}

	if (q - p < inlen)
	{
		x->title = calloc(1, (q - p) + 1);
		memmove(x->title, p, q - p);
	}

	assert(x->title != NULL);

	return 0;
}

static int ReadAuthor(uIM_GuideHeader *x)
{
	char *p;
	char *q;

	q = SkipLines((char*) inbuff, 1);
	p = q;
	assert(q!=NULL);
	while (IsCRLF(*q) == 0 && q - (char*) inbuff < inlen)
	{
		q++;
	}

	if (q - (char*) inbuff < inlen)
	{
		x->author = calloc(1, (q - p) + 1);
		memmove(x->author, p, q - p);
	}

	assert(x->author != NULL);

	return 0;
}

static int ReadRev(uIM_GuideHeader *x)
{
	char *p;
	char *q;

	q = SkipLines((char*) inbuff, 2);
	p = q;
	assert(q!=NULL);
	while (IsCRLF(*q) == 0 && q - (char*) inbuff < inlen)
	{
		q++;
	}

	if (q - (char*) inbuff < inlen)
	{
		x->revision = calloc(1, (q - p) + 1);
		memmove(x->revision, p, q - p);
	}

	assert(x->author != NULL);

	return 0;
}

static char* SkipToEOL(char *q)
{
	while (IsCRLF(*q) == 0 && q - (char*) inbuff < inlen)
	{
		q++;
	}

	if (q - (char*) inbuff < inlen)
	{
		while (IsCRLF(*q) == 1 && q - (char*) inbuff < inlen)
		{
			q++;
		}
	}

	return q;
}

static int IsHdr(char *p)
{
	if (p[0] == '%')
		return 0;
	else
		return 1;
}

static char* BustNodeChops(uIM_Node *node, char *p)
{
	char *q;
	int count;

	q = p;


	// skip initial
	q++;
    count = 0;
	while (IsCRLF(*q) == 0)
	{
		if (*q == '%')
			count++;

		q++;
	}

	node->node_count = count;
	node->nodes = (char**) calloc(count, sizeof(char*));

	p++;

	for (count = 0; count < node->node_count; count++)
	{
		q = p;

		while (IsCRLF(*q) == 0 && *q != '%')
			q++;

		node->nodes[count] = calloc(1, (q - p) + 4);
		memmove(node->nodes[count], p, (q - p));

		p = q;

		assert(*p == '%');
		if (*p == '%')
			;
		p++;
	}

	return p;
}

static int ParseNodes(uIM_GuideHeader *x)
{
	char *p;
	char *end;
	int line;
	int fixed;

	end = (char*) inbuff + inlen;

	p = SkipLines((char*) inbuff, 3);
	assert(p != NULL);

	if (memcmp(p, "----", 4) != 0)
	{
		printf("malformed document, missing boundary line 4\n");
		return 1;
	}

	p = SkipToEOL(p);
	line = 5;

	while (p < end)
	{
		if (IsHdr(p) == 0)
		{
			char *q;
			int flag;

			uIM_Node *n = AllocNode();

			p = BustNodeChops(n, p);

			if (p[0] == 0x0D && p[1] == 0x0A)
			{
				line++;
				p += 2;
			}
			else if (p[0] == 0x0A || p[0] == 0x0D)
			{
				line++;
				p++;
			}

			// work node data...
			q = p;

			fixed = 0;
			flag = 0;

			while (flag == 0 && p < end)
			{
				if (p[0] == 0x0D && p[1] == 0x0A)
				{
					line++;
					p += 2;
				}
				else if (p[0] == 0x0A || p[0] == 0x0D)
				{
					line++;
					p++;
				}

				if (memcmp(p, "{{{", 3) == 0)
				{
					fixed += 1;
					p += 3;
				}
				else if (memcmp(p, "}}}", 3) == 0)
				{
					fixed -= 1;
					p += 3;
				}

				if (fixed == 0)
				{
					// skip over special marker
					if (memcmp(p, "%%", 2) == 0)
					{
						char *xxx = p;

						p += 2;

						while (p < end && !(memcmp(p, "%%", 2) == 0) && *p != 0x0A && *p != 0x0)
						{
							p++;
						}

						if(*p == 0x0A || *p == 0x0)
						{
							*p = 0x0;
							printf("Bad token line at line %i, %s\n", line, xxx);
							exit(0);
						}

						if (p < end)
							p += 2;
					}
					else
					{
						if (*p != '%')
							p++;
					}

				}
				else
					p++;

				if (fixed == 0 && (p[0] == '%' && p[1] != '%'))
					flag = 1;
			}

			// end of node data
			n->data = calloc(1, (p - q) + 4);
			memmove(n->data, q, p - q);

			dlist_ins(x->lstNodes, n);
		}
		else
		{
			printf("Missing header on line %i\n", line);
			exit(0);
		}
	}

	return 0;
}

static uint8_t* reparse(uint8_t *in, uint32_t inlen)
{
	uint8_t *xbuff;
	int idx;

	xbuff = calloc(1, inlen + 4);
	idx = 0;

	while (*in != 0)
	{
		if (in[0] == 0x0D && in[1] == 0x0A)
		{
			xbuff[idx++] = 0x0A;
			in += 2;
		}
		else if (in[0] == 0x0D)
		{
			xbuff[idx++] = 0x0A;
			in += 1;
		}
		else
		{
			xbuff[idx++] = in[0];
			in += 1;
		}
	}

	xbuff[idx++] = 0;

	return xbuff;
}

static void WriteToDisk(uIM_GuideHeader *x, char *ofn)
{
	FILE *fp;
	struct udtODS_GuideHeader *oh;
	struct udtODS_GuideEntry *on;
	int ohlen;
	uint8_t *p;
	uint32_t j;

	uint32_t *noff;

	DLElement *e;

	remove(ofn);
	fp = fopen(ofn, "wb");

	if (fp == NULL)
	{
		printf("Could not create output file %s\n", ofn);
		return;
	}

	ohlen = dlist_size(x->lstNodes) * 4;
	ohlen += sizeof(struct udtODS_GuideHeader);
	ohlen += strlen(x->title) + 1 + strlen(x->author) + 1 + strlen(x->revision) + 1;

	oh = calloc(1, ohlen);
	memset(oh, 0x0, ohlen);
	p = (uint8_t*) oh;

	oh->flags = comp_type;

	oh->magic = GUIDE_MAGIC;
	oh->version = 0x0100;
	oh->node_count = dlist_size(x->lstNodes);
	oh->meta_offset = sizeof(struct udtODS_GuideHeader);
	oh->meta_size = strlen(x->title) + 1 + strlen(x->author) + 1 + strlen(x->revision) + 1;

	j = oh->meta_offset;
	memmove(p + j, x->title, strlen(x->title) + 1);
	j += strlen(x->title) + 1;
	memmove(p + j, x->author, strlen(x->author) + 1);
	j += strlen(x->author) + 1;
	memmove(p + j, x->revision, strlen(x->revision) + 1);
	j += strlen(x->revision) + 1;

	oh->node_offset = j;
	oh->header_size = ohlen;

	noff = (uint32_t*) (j + p);


	// update offsets of data...
	fseek(fp, 0x0L, SEEK_SET);
	fwrite(oh, 1, ohlen, fp);

	e = dlist_head(x->lstNodes);
	while (e != NULL)
	{
		int i;
		int j;
		uIM_Node *n;
		uint8_t *p;

		uint32_t inlen;

		uint8_t *buff;
		uint8_t *buff2;
		uint32_t clen;

		n = dlist_data(e);
		e = dlist_next(e);

		buff2 = reparse(n->data, strlen((char*) n->data) + 1);
        clen = 0;
		inlen = 4 * strlen((char*) buff2);
		if (inlen < 16384)
			inlen = 16384;

		buff = malloc(inlen);
		in_total += strlen((char*) buff2);

		switch (comp_type)
		{
			case F_LZSS_COMPRESSION:
				clen = lzss_encode(buff, inlen, buff2, strlen((char*) buff2));
				break;


#ifndef __MINGW_H
			case F_BZIP2_COMPRESSION:
				BZ2_bzBuffToBuffCompress((char*) buff, &inlen, (char*) buff2, strlen((char*) buff2), 9, 0, 30);
				clen = inlen;
				break;
#endif
			case F_GZIP_COMPRESSION:
				{
					unsigned long foo = inlen;
					compress2((uint8_t*) buff, &foo, (uint8_t*) buff2, strlen((char*) buff2), 9);
					clen = foo;
				}
				break;
			case F_NONE:
				memmove(buff, buff2, strlen((char*) buff2));
				clen = strlen((char*) buff2);
				break;
		}

		free(buff2);

		out_total = out_total + (uint64_t) clen;


		//assert(clen < strlen((char*)n->data)*2);
		assert(clen != 0);

		j = sizeof(struct udtODS_GuideEntry);
		for (i = 0; i < n->node_count; i++)
			j += 1 + strlen((char*) n->nodes[i]);

		j += clen;

		on = calloc(1, j + 4);
		p = (uint8_t*) on;

		on->header_count = n->node_count;
		on->length = j;
		on->orig_length = strlen((char*) n->data);

		printf("Node : ");
		j = sizeof(struct udtODS_GuideEntry);
		for (i = 0; i < n->node_count; i++)
		{
			memmove(p + j, (char*) n->nodes[i], strlen((char*) n->nodes[i]) + 1);
			j += strlen((char*) n->nodes[i]) + 1;
			if (i > 0)
				printf("->");
			printf("%s", n->nodes[i]);
		}

		if (clen > on->orig_length)
			printf("  WARN in = %i out = %i, (%i%%)\n", on->orig_length, clen, (clen * 100) / on->orig_length);
		else
			printf("  in = %i out = %i, (%i%%)\n", on->orig_length, clen, (clen * 100) / on->orig_length);

		memmove(p + j, buff, clen);
		j += clen;

		*noff++ = ftell(fp);

		fwrite(on, 1, j, fp);

		free(buff);
		free(on);
	}

	// update offsets of data...
	fseek(fp, 0x0L, SEEK_SET);
	fwrite(oh, 1, ohlen, fp);

	fclose(fp);
}

static void syntax(void)
{
	printf("syntax: gcomp {opts} inputfile outputfile\n");
	printf("opts are;\n\n");
	printf("--lzss - lzss compression\n");
#ifndef __MINGW_H
	printf("--bzip2 - bzip2 compression\n");
#endif
	printf("--gzip - gzip (default) compression\n");
	printf("--none - no compression\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	uIM_GuideHeader *hdr;
	int i;

	printf("GuideCompiler v0.1\n");

	comp_type = F_GZIP_COMPRESSION;
	for (i = 1; i < argc; i++)
	{
#ifndef __MINGW_H
		if (strcmp(argv[i], "--bzip2") == 0)
			comp_type = F_BZIP2_COMPRESSION;
		else
#endif
		if (strcmp(argv[i], "--gzip") == 0)
			comp_type = F_GZIP_COMPRESSION;
		else if (strcmp(argv[i], "--lzss") == 0)
			comp_type = F_LZSS_COMPRESSION;
		else if (strcmp(argv[i], "--none") == 0)
			comp_type = F_NONE;
		else
		{
			if (in_file == NULL && out_file == NULL)
				in_file = argv[i];
			else if (in_file != NULL && out_file == NULL)
				out_file = argv[i];
			else
			{
				syntax();
			}
		}
	}

	if (in_file == NULL || out_file == NULL)
		syntax();

	if (ReadInputFile(in_file) == 0)
	{
		in_total = 0;
		out_total = 0;

		hdr = AllocGuide();

		ReadTitle(hdr);
		ReadAuthor(hdr);
		ReadRev(hdr);

		ParseNodes(hdr);
		WriteToDisk(hdr, out_file);
		printf("Compressed : in %i, out %i (%i%%)\n", in_total, out_total, (out_total * 100) / in_total);
	}

	return 0;
}
