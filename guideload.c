#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <zlib.h>

#include "dlist.h"
#include "logwrite.h"

#include "guideheader.h"


struct udtRecord
{
	int type;
	int length;
	int olength;
	uint8_t *data;
};

static uHelpSection* LookupSection(uHelpFile *hlp, char *section)
{
	DLElement *e;

	e = dlist_head(hlp->lstSections);

	while (e != NULL)
	{
		uHelpSection *sect;

		sect = dlist_data(e);
		e = dlist_next(e);

		if (strcmp(sect->name, section) == 0)
			return sect;
	}

	return NULL;
}

void FreeHelpPage(uHelpPage *p)
{
	int i;

	free(p->name);
	for (i = 0; i < p->line_count; i++)
		free(p->lines[i]);
	free(p->lines);

	if (p->_links != NULL)
	{
		for (i=0; i < p->link_count; i++)
		{
			if (p->_links[i]->link != NULL)
				free(p->_links[i]->link);

			if (p->_links[i]->show != NULL)
				free(p->_links[i]->show);

			free(p->_links[i]);
		}
		free(p->_links);
	}

	free(p);
}

static void newline(uHelpPage *page)
{
	int col;

	assert(page->lines != NULL);

	page->lines = realloc(page->lines, (page->line_count + 1) * sizeof(uint16_t*));
	assert(page->lines != NULL);

	page->lines[page->line_count] = calloc(page->width, sizeof(uint16_t));

	for (col = 0; col < page->width; col += 1)
		page->lines[page->line_count][col] = 0x0020;

	page->line_count += 1;
}


uHelpPage* HelpReflowPage(uHelpFile *hlp, char *section, int width, int link)
{
	uHelpSection *sect;
	uHelpPage *page;
	uint8_t flags;
	int last_id = 0;

	int stack_depth;
	uint8_t stack[256];

	DLElement *e;

	sect = LookupSection(hlp, section);
	if (sect == NULL)
		return NULL;

	page = calloc(1, sizeof(uHelpPage));
	page->name = strdup(section);
	page->line_count = 0;
	page->lines = calloc(1, 1 * sizeof(uint16_t*));
	page->width = width;
	page->highlight_link = link;
	page->link_count = 0;

	if (page->highlight_link < 0)
		page->highlight_link = 1;

	flags = 0;
	stack_depth = 0;
	stack[stack_depth] = 0;

	e = dlist_head(sect->lstLines);
	while (e != NULL)
	{
		char *x;
		char *z;

		uint16_t *p;
		int col;

		char *last_x;
		uint16_t *last_p;
		int last_flags;

		x = dlist_data(e);
		e = dlist_next(e);

		newline(page);
		p = page->lines[page->line_count - 1];

		col = 0;
		z = x;

		last_x = x;
		last_p = p;
		last_flags = flags;

		while (*x != 0x0)
		{
			if (x[0] == '\\')
			{
				if (strncmp(x, "\\emph{", 6) == 0)
				{
					assert(stack_depth < 255);

					stack[stack_depth] = flags;
					stack_depth += 1;

					flags |= HLP_F_EMPH;
					x += 6;
				}
				else if (strncmp(x, "\\bold{", 6) == 0)
				{
					assert(stack_depth < 255);

					stack[stack_depth] = flags;
					stack_depth += 1;

					flags |= HLP_F_BOLD;
					x += 6;
				}
				else if (strncmp(x, "\\link{", 5) == 0)
				{
					char *p;
					char *q;

					stack_depth += 1;
					stack[stack_depth] = flags;
					x += 6;

					if (last_id % 2 == 0)
						flags = HLP_F_LINK1;
					else
						flags = HLP_F_LINK2;

					page->link_count += 1;
					page->_links = realloc(page->_links, sizeof(uHelpLink*) * page->link_count);
					page->_links[page->link_count - 1] = calloc(1, sizeof(uHelpLink));

					page->_links[page->link_count - 1]->col = col;
					page->_links[page->link_count - 1]->row = page->line_count;
					page->_links[page->link_count - 1]->id = last_id ++;
					page->_links[page->link_count - 1]->display_length = 0;
					page->_links[page->link_count - 1]->link = NULL;
					page->_links[page->link_count - 1]->show = NULL;

					p = x;
					q = x;
					while (*p != '}' && *p != 0x0)
					{
						if (*p == '|')
						{
							page->_links[page->link_count - 1]->link = calloc(1, 4+(p - x));
							memmove(page->_links[page->link_count - 1]->link, x, p - x);
							x = p + 1;
							q = x;
						}

						p++;
					}

					if (page->_links[page->link_count - 1]->link == NULL)
					{
						page->_links[page->link_count - 1]->link = calloc(1, 4+(p - x));
						memmove(page->_links[page->link_count - 1]->link, x, p - x);
					}

					page->_links[page->link_count - 1]->show = calloc(1, 4+(p - q));
					memmove(page->_links[page->link_count - 1]->show, q, p - q);
				}
				else
				{
					char c;

					c = x[20];
					x[20] = 0;

					fprintf(stderr, "unkown at %s\n", x);
					fflush(stderr);

					x[20] = c;

					exit(0);
				}
			}
			else if (*x == '}')
			{
				assert(stack_depth > 0);

				stack_depth -= 1;
				flags = stack[stack_depth];

				x++;
			}
			else
			{
				switch ((*x & 0xFF))
				{
					case 0x0A:
					case 0x0D:
						x++;
						break;

					default:
						{
							*p = ((*x & 0xFF) | (flags << 8));

							if ((*x & 0xFF) == 0x20)
							{
								last_p = p;
								last_x = x;
								last_flags = flags;
							}


							if ((flags & (HLP_F_LINK1 | HLP_F_LINK2)) != 0)
							{
								assert(page->_links != NULL);
								assert(page->link_count > 0);
								page->_links[page->link_count - 1]->display_length += 1;
							}

							p++;
							x++;
							col++;

							if (col == page->width)
							{
								int last_flags_min_link;

								last_flags_min_link = last_flags & ~(HLP_F_LINK1 | HLP_F_LINK2);

								while (last_p < p)
									*last_p++ = ((0x00) | (last_flags_min_link << 8));

								x = last_x;
								flags = last_flags;

								while (*x == 0x20)
									x++;

								newline(page);
								p = page->lines[page->line_count - 1];
								col = 0;
							}
						}
						break;
				}
			}
		}
	}

	return page;
}

static struct udtRecord* ReadRecord(FILE *fp)
{
	struct udtRecord *rec;

	rec = calloc(1, sizeof(struct udtRecord));

	rec->type = fgetc(fp);
	rec->length = (fgetc(fp) << 8) + fgetc(fp);
	rec->olength = rec->length;

	if (rec->type >= 0x20)
		rec->olength = (fgetc(fp) << 8) + fgetc(fp);

	rec->data = calloc(1, rec->length + 7);
	fread(rec->data, 1, rec->length, fp);

	if (rec->type >= 0x20)
	{
		unsigned long foo;
		uint8_t *buff;

		buff = calloc(1, rec->olength + 7);
		foo = rec->olength;
		uncompress((uint8_t*) buff, &foo, (uint8_t*) rec->data, rec->length);

		free(rec->data);
		rec->data = buff;
		rec->length = rec->olength;

		rec->type -= 0x20;
	}

	return rec;
}

static void FreeSection(void *data)
{
	uHelpSection *h;

	h = data;

	dlist_destroy(h->lstLines);
	free(h->lstLines);
	free(h->name);
	free(h);
}

void FreeBreadCrumb(void *data)
{
	uHelpBreadCrumb *b = data;

	if (b->prev_link != NULL)
		free(b->prev_link);

	free(b);
}

void FreeHelpFile(uHelpFile *hlp)
{
	if (hlp == NULL)
		return;

	FreeDList(hlp->lstSections);
	FreeDList(hlp->lstBreadCrumbs);

	if (hlp->author != NULL)
		free(hlp->author);
	if (hlp->revision != NULL)
		free(hlp->revision);
	if (hlp->title != NULL)
		free(hlp->title);
	free(hlp);
}

static void FreeLine(void *data)
{
	free(data);
}

static int IsValidHeader(FILE *fp)
{
	int x;

	x = fgetc(fp) << 8;
	x += fgetc(fp);

	if (x == GUIDE_MAGIC)
		return 0;
	else
		return 1;
}

static void FreeRecord(struct udtRecord *rec)
{
	free(rec->data);
	free(rec);
}

uHelpFile* LoadHelpFile(char *fn)
{
	uHelpFile *hlp = NULL;

	FILE *fp;
	uint32_t mlen;

	uHelpSection *sect = NULL;

	hlp = calloc(1, sizeof(uHelpFile));

	hlp->lstSections = NewDList(FreeSection);
	hlp->lstBreadCrumbs = NewDList(FreeBreadCrumb);

	fp = fopen(fn, "rb");

	if (fp != NULL)
	{
		fseek(fp, 0x0L, SEEK_END);
		mlen = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);

		if (IsValidHeader(fp) == 0)
		{
			while (ftell(fp) < mlen && fp != NULL)
			{
				struct udtRecord *rec;

				rec = ReadRecord(fp);
				switch (rec->type)
				{
					case HLP_AUTHOR:
						hlp->author = strdup((char*) rec->data);
						FreeRecord(rec);
						break;
					case HLP_VERSION:
						hlp->revision = strdup((char*) rec->data);
						FreeRecord(rec);
						break;
					case HLP_TITLE:
						hlp->title = strdup((char*) rec->data);
						FreeRecord(rec);
						break;

					case HLP_SECTION:
						sect = calloc(1, sizeof(uHelpSection));
						sect->name = strdup((char*) rec->data);
						sect->lstLines = malloc(sizeof(DList));
						dlist_init(sect->lstLines, FreeLine);

						dlist_ins(hlp->lstSections, sect);

						FreeRecord(rec);
						break;

					case HLP_LINE:
						assert(sect != NULL);

						dlist_ins(sect->lstLines, strdup((char*)rec->data));
						FreeRecord(rec);
						break;

					default:
						// unknown record type
						LogInfo("unknown type %i\n", rec->type);
						FreeRecord(rec);
						break;
				}
			}
		}
		else
			LogInfo("Invalid help header\n");

		fclose(fp);
	}
	else
		LogInfo("Could not open %s\n", fn);

	return hlp;
}


