#include "headers.h"

#include "lzss.h"

#ifndef __MINGW_H
#include <bzlib.h>
#endif
#include <zlib.h>

#include "guideheader.h"

static void FreeNode(void *x)
{
	uIM_Node *n = x;
	int i;

	for (i = 0; i < n->node_count; i++)
		free(n->nodes[i]);

	free(n->nodes);
	free(n->data);
	free(n);
}

uIM_GuideHeader* LoadGuide(char *fn)
{
	FILE *fp;
	struct udtODS_GuideHeader *oh;
	struct udtODS_GuideEntry *on;
	struct udtODS_GuideHeader oh1;

	uIM_GuideHeader *hdr;
	uIM_Node *node;

	uint32_t *nodes;
	char *p;
	int i;

	fp = fopen(fn, "rb");
	if (fp == NULL)
		return NULL;

	fread(&oh1, 1, sizeof(struct udtODS_GuideHeader), fp);

	if (oh1.magic != GUIDE_MAGIC)
	{
		fclose(fp);
		printf("bad header magic\n");
		return NULL;
	}

	oh = calloc(1, oh1.header_size);

	fseek(fp, 0x0L, SEEK_SET);
	fread(oh, 1, oh1.header_size, fp);

	p = ((char*) oh) + oh->meta_offset;
	nodes = (uint32_t*) (((uint8_t*) oh) + oh->node_offset);

	hdr = calloc(1, sizeof(uIM_GuideHeader));
	hdr->lstNodes = malloc(sizeof(DList));
	dlist_init(hdr->lstNodes, FreeNode);

	hdr->flags = oh->flags;
	hdr->version = oh->version;

	hdr->title = strdup(p);
	while (*p++ != 0x0)
		;

	hdr->author = strdup(p);
	while (*p++ != 0x0)
		;

	hdr->revision = strdup(p);
	while (*p++ != 0x0)
		;

	on = malloc(1024 * 65);

	for (i = 0; i < oh->node_count; i++)
	{
		int k;
		char *q;
		int dlen;
		uint8_t *buff;
		uint32_t out_len;

		node = calloc(1, sizeof(uIM_Node));

		fseek(fp, nodes[i], SEEK_SET);
		fread(on, 1, 1024 * 65, fp);
		q = ((char*) on) + sizeof(struct udtODS_GuideEntry);

		dlen = sizeof(struct udtODS_GuideEntry);

		buff = calloc(1, on->orig_length + 4);

		node->node_count = on->header_count;
		node->nodes = calloc(node->node_count, sizeof(char*));

		for (k = 0; k < on->header_count; k++)
		{
			node->nodes[k] = strdup(q);

			dlen += strlen(q) + 1;
			while (*q++ != 0x0)
				;
		}

		if ((oh->flags & F_COMPRESSION_MASK) == F_LZSS_COMPRESSION)
		{
			// on->length as last param is incorrect, we need to sub size of chops + header
			lzss_decode(buff, on->orig_length, (uint8_t*) q, on->length - dlen);
		}
#ifndef __MINGW_H
		else if ((oh->flags & F_COMPRESSION_MASK) == F_BZIP2_COMPRESSION)
		{
			out_len = on->orig_length;
			// on->length as last param is incorrect, we need to sub size of chops + header
			BZ2_bzBuffToBuffDecompress((char*) buff, &out_len, (char*) q, on->length - dlen, 0, 1);
		}
#endif
		else if ((oh->flags & F_COMPRESSION_MASK) == F_GZIP_COMPRESSION)
		{
			unsigned long foo;

			foo = on->orig_length;
			// on->length as last param is incorrect, we need to sub size of chops + header
			uncompress((uint8_t*) buff, &foo, (uint8_t*) q, on->length - dlen);
		}
		else if ((oh->flags & F_COMPRESSION_MASK) == F_LZMA_COMPRESSION)
		{
		}
		else if ((oh->flags & F_COMPRESSION_MASK) == F_RLE_COMPRESSION)
		{
		}
		else
		{
			out_len = on->orig_length;
			memmove(buff, q, out_len);
		}

		node->length = on->orig_length;
		node->data = buff;
		strcat((char*) node->data, "\x0A");

		dlist_ins(hdr->lstNodes, node);
	}

	free(oh);
	free(on);

	return hdr;
}

static uIM_Node* FindNode(uIM_GuideHeader *hdr, int depth, char **name)
{
	DLElement *e;

	e = dlist_head(hdr->lstNodes);

	while (e != NULL)
	{
		uIM_Node *n;

		n = dlist_data(e);
		e = dlist_next(e);

		if (n->node_count == depth)
		{
			int i;
			int flag;

			for (i = 0, flag = 0; i < n->node_count; i++)
			{
				if (stricmp(n->nodes[i], name[i]) == 0)
					flag++;
				else
					break;
			}

			if (flag == n->node_count)
				return n;
		}
	}

	return NULL;
}

static uint8_t *new_line(int length, char *x)
{
	uint8_t *z = calloc(1, length + 1);
	memmove(z, x, length);

	return z;
}

static uIM_GuideLink* BustLink(char *p)
{
	uIM_GuideLink *l;
	int c;

	l = calloc(1, sizeof(uIM_GuideLink));
	l->nodes = calloc(32, sizeof(char*));

	if (memcmp(p, "%%LINK%", 7) == 0)
	{
		int len = 0;
		char *q;
		p += 7;

		while (p[0] != '%' && p[1] != '%')
		{
			q = p;
			while (*p != '%')
				p++;

			l->nodes[l->node_count++] = (char*) new_line(p - q, q);
			len += p - q;

			p++;
		}

		l->link = malloc(4 + strlen(l->nodes[l->node_count-1]));
		sprintf(l->link, "%%%s%%", l->nodes[l->node_count - 1]);

		l->canonical_link = calloc(1, 4 + l->node_count + len);

		strcat(l->canonical_link, "%");
		for (c = 0; c < l->node_count; c++)
		{
			strcat(l->canonical_link, l->nodes[c]);
			strcat(l->canonical_link, "%");
		}
	}

	return l;
}

static void FreeLine(void *x)
{
	free(x);
}

static void FreeLink(void *n)
{
	uIM_GuideLink *l = n;
	int i;

	free(l->canonical_link);
	free(l->link);
	for (i = 0; i < l->node_count; i++)
		free(l->nodes[i]);

	free(l->nodes);
	free(l);
}

void FreePage(uIM_GuidePage *page)
{
	dlist_destroy(page->lstLines);
	dlist_destroy(page->lstLinks);

	free(page->lstLines);
	free(page->lstLinks);
	free(page);
}

static uIM_GuidePage *FormatPage(uIM_GuidePage *page_data, int page_width)
{
	DLElement *e;
	char *pp;
	int fixed;
	int i;
	int wide;
	uIM_GuideLine *line;
	uIM_GuideLine *ln;
	int style;
	int flags;
	int old_flags;

	DList *lstNewLines;

	char *start_line;

	lstNewLines = malloc(sizeof(DList));
	dlist_init(lstNewLines, FreeLine);

	e = dlist_head(page_data->lstLines);
	line = dlist_data(e);

	ln = calloc(1, sizeof(uIM_GuideLine));
	ln->flags = 0;
	ln->text = (uint8_t*)strdup((char*)line->text);
	e = dlist_next(e);

	dlist_ins(lstNewLines, ln);

	fixed = 0;
	line = dlist_data(e);

	start_line = calloc(1, 4096);

	i = 0;
	style = STYLE_NORMAL;
	flags = 0;
	old_flags = 0;
	while (e != NULL)
	{
		line = dlist_data(e);
		pp = (char*) line->text;

		wide = 0;

		while (*pp != 0 && wide < page_width)
		{
			int wlen;
			int rlen;

			// calculate word length.
			wlen = 0; // word length - codes
			rlen = 0; // total word length + codes

			if (pp[wlen] == ' ')
			{
				rlen++;
				wlen++;
			}

			while (pp[wlen] != 0x0 && pp[wlen] != 0x0A && pp[wlen] != ' ')
			{
				switch (pp[wlen])
				{
					case '{':
					case '}':
						if (memcmp(pp + wlen, "{{{", 3) == 0)
						{
							fixed += 1;
							wlen += 3;
							flags += 0x100;
							if (fixed > 1)
								rlen += 3;
						}
						else if (memcmp(pp + wlen, "}}}", 3) == 0)
						{
							fixed -= 1;
							wlen += 3;
							flags -= 0x100;
							if (fixed > 0)
								rlen += 3;
						}
						else
						{
							wlen++;
							rlen++;
						}
						break;

					case '@':
					case '~':
					case '^':
						if (fixed > 0)
							rlen++;
						else
						{
							switch (pp[wlen])
							{
								case '~':
									if (fixed == 0)
									{
										if ((flags & 0xFF) == 0)
											flags |= eLF_Bold;
										else
											flags &= 0xFF00;
									}
									break;
								case '^':
									if (fixed == 0)
									{
										if ((flags & 0xFF) == 0)
											flags |= eLF_Reverse;
										else
											flags &= 0xFF00;
									}
									break;
								case '@':
									if (fixed == 0)
									{
										if ((flags & 0xFF) == 0)
											flags |= eLF_Underline;
										else
											flags &= 0xFF00;
									}
									break;
							}
						}
						wlen++;
						break;

					default:
						wlen++;
						rlen++;
						break;
				}
			}


			// draw it on screen...
			if (wide + rlen > page_width)
			{
				// new line
				i += 1;
				wide = 0;

				ln = calloc(1, sizeof(uIM_GuideLine));
				ln->flags = old_flags;
				ln->text = (uint8_t*) strdup(start_line);
				start_line[0] = 0;

				old_flags = flags;

				dlist_ins(lstNewLines, ln);

				if (fixed == 0)
				{
					while (*pp == ' ')
						pp++;
				}

				strncat(start_line, pp, wlen);
				pp += wlen;
				wide += rlen;
			}
			else
			{
				strncat(start_line, pp, wlen);
				pp += wlen;
				wide += rlen;
			}
		}

		//if (start_line[0] != 0)
		{
			ln = calloc(1, sizeof(uIM_GuideLine));
			ln->flags = old_flags;
			ln->text = (uint8_t*) strdup(start_line);
			start_line[0] = 0;
			dlist_ins(lstNewLines, ln);

			old_flags = flags;

			i += 1;
		}

		e = dlist_next(e);
	}

	dlist_destroy(page_data->lstLines);
	free(page_data->lstLines);
	page_data->lstLines = lstNewLines;

	return page_data;
}


uIM_GuidePage* ReflowPage(uIM_GuideHeader *hdr, int depth, char **titles, int page_width)
{
	uIM_Node *node;
	char *p;
	char *line_buffer;
	int idx;
	int fixed;
	uIM_GuideLine *ln;

	int last_flags;
	int flags;

	uIM_GuidePage *rv;

	node = FindNode(hdr, depth, titles);

	if (node == NULL)
		return NULL;

	rv = calloc(1, sizeof(uIM_GuidePage));
	rv->lstLinks = malloc(sizeof(DList));
	rv->lstLines = malloc(sizeof(DList));

	dlist_init(rv->lstLinks, FreeLink);
	dlist_init(rv->lstLines, FreeLine);

	line_buffer = malloc(8192);

	p = calloc(1, 4096);
	for (idx = 0; idx < node->node_count; idx++)
	{
		if (idx > 0)
			strcat(p, "/");
		strcat(p, node->nodes[idx]);
	}

	ln = calloc(1, sizeof(uIM_GuideLine));
	ln->flags = 0;
	ln->text = (uint8_t*) strdup(p);

	dlist_ins(rv->lstLines, ln);
	free(p);

	p = (char*) node->data;

	last_flags = 0;

	idx = 0;
	fixed = 0;
	flags = 0;
	last_flags = 0;
	while (*p != 0)
	{
		switch (*p)
		{
			case '}':
			case '{':
				if (memcmp(p, "{{{", 3) == 0)
				{
					fixed += 1;
					flags += 0x100;

					line_buffer[idx++] = *p++;
					line_buffer[idx++] = *p++;
					line_buffer[idx++] = *p++;
					line_buffer[idx] = 0;

					if (fixed == 1)
						flags = (flags & 0xFF00) + eLF_Fixed;
				}
				else if (memcmp(p, "}}}", 3) == 0)
				{
					fixed -= 1;
					flags -= 0x100;

					line_buffer[idx++] = *p++;
					line_buffer[idx++] = *p++;
					line_buffer[idx++] = *p++;
					line_buffer[idx] = 0;

					if (fixed == 0)
						flags = (flags & 0xFF00) | eLF_None;
				}
				else
				{
					line_buffer[idx++] = *p++;
					line_buffer[idx] = 0;
				}
				break;

			case 0x0A:
				if (fixed >= 1 || (p[1] == 0x0A && idx > 0))
				{
					while (*p == 0x0A)
					{
						ln = calloc(1, sizeof(uIM_GuideLine));
						ln->flags = last_flags;
						ln->text = new_line(idx, line_buffer);
						ln->length = strlen((char*)ln->text);
						//fprintf(stderr, "line(%04X) : %s\n", ln->flags, ln->text);
						dlist_ins(rv->lstLines, ln);
						last_flags = flags;
						idx = 0;
						line_buffer[idx] = 0;
						p++;
					}
				}
				else
				{
					line_buffer[idx++] = ' ';
					line_buffer[idx] = 0;
					p++;
				}
				break;

			case '%':
				if (fixed == 0)
				{
					if (memcmp(p + 1, "%LINK%", 6) == 0)
					{
						uIM_GuideLink *link;

						link = BustLink(p);
						link->row = dlist_size(rv->lstLines);
						link->col = idx;

						dlist_ins(rv->lstLinks, link);

						memmove(line_buffer + idx, link->link, strlen(link->link));
						idx += strlen(link->link);
						line_buffer[idx] = 0;

						p += 2;
						while (!(p[0] == '%' && p[1] == '%'))
							p++;

						p += 2;
					}
					else if (memcmp(p + 1, "%LIST%", 6) == 0)
					{
						while (*p != 0x0A && *p != 0x0)
						{
							p++;
						}
					}
					else
					{
						line_buffer[idx++] = *p++;
						line_buffer[idx] = 0;
					}
				}
				else
				{
					line_buffer[idx++] = *p++;
					line_buffer[idx] = 0;
				}
				break;

			default:
				{
					switch (*p)
					{
						case '~':
							if (fixed == 0)
							{
								if ((flags & 0xFF) == 0)
									flags |= eLF_Bold;
								else
									flags &= 0xFF00;
							}
							break;
						case '^':
							if (fixed == 0)
							{
								if ((flags & 0xFF) == 0)
									flags |= eLF_Reverse;
								else
									flags &= 0xFF00;
							}
							break;
						case '@':
							if (fixed == 0)
							{
								if ((flags & 0xFF) == 0)
									flags |= eLF_Underline;
								else
									flags &= 0xFF00;
							}
							break;
					}

					line_buffer[idx++] = *p++;
					line_buffer[idx] = 0;
				}
				break;
		}
	}

	if (idx > 0)
	{
		ln = calloc(1, sizeof(uIM_GuideLine));
		ln->flags = last_flags;
		ln->text = new_line(idx, line_buffer);
		ln->length = strlen((char*)ln->text);
		//fprintf(stderr, "line(%04X) : %s\n", ln->flags, ln->text);
		dlist_ins(rv->lstLines, ln);
		last_flags = flags;
	}

	free(line_buffer);

	return FormatPage(rv, page_width);
}

void FreeGuide(uIM_GuideHeader *g)
{
	if (g->author != NULL)
		free(g->author);
	if (g->revision != NULL)
		free(g->revision);
	if (g->title != NULL)
		free(g->title);

	dlist_destroy(g->lstNodes);
	free(g->lstNodes);

	memset(g, 0x0, sizeof(uIM_GuideHeader));

	free(g);
}


