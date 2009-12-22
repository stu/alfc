#include "headers.h"
#include <zlib.h>

#include "guideheader.h"

struct udtRecord
{
	int type;
	int length;
	int olength;
	uint8_t *data;
};

static void status_msg(uWindow *w, char *strX, ...)
{
	va_list args;
	char *z;

	z = malloc(4096);
	assert(z != NULL);
	assert(strlen(strX) < 1024);

	if (strX != NULL)
	{
		va_start(args, strX);
		vsprintf(z, strX, args);
		va_end(args);
	}
	else
	{
		*z = 0;
	}

	w->screen->set_style(STYLE_HELP_TITLE);
	w->screen->set_cursor(w->offset_row + w->height, w->offset_col + 2);
	w->screen->print(z);

	free(z);
}

static void display_char(uWindow *w, char c)
{
	char x[2];

	x[0] = c;
	x[1] = 0;
	w->screen->print((char*) &x);
}

static void help_draw_window(uWindow *w, uHelpPage *page_data)
{
	char *buff;
	char *q, *oq;;

	w->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->window_clear(w);
	w->screen->set_style(STYLE_HELP_TITLE);
	w->screen->draw_border(w);
	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	assert(page_data->name != NULL);

	buff = malloc( strlen(page_data->name) + 5);

	q = strchr(page_data->name, ':');
	if (q != NULL)
	{
		do
		{
			q++;
			oq = q;
			q = strchr(q, ':');
		}while (q != NULL);
		q = oq;
	}
	else
		q = page_data->name;

	sprintf(buff, "%s ", q);
	w->screen->print(buff);
	free(buff);
}

static void draw_percentage(uWindow *w, uHelpPage *page)
{
	int line;
	char buff[8];

	line = page->line_count;
	line -= w->height - 2;
	if (line < 0)
		line = 100;
	else
	{
		line = (w->top_line*100) / line;
	}

	sprintf(buff, " %3i%% ", line);

	w->screen->set_style(STYLE_HELP_TITLE);
	w->screen->set_cursor(w->offset_row + w->height, w->offset_col + w->width - 7);
	w->screen->print(buff);
}

static void help_draw_page(uWindow *w, uHelpPage *page_data)
{
	uint16_t *pp;
	int wide;
	int style = STYLE_HELP_NORMAL;
	int line;
	int width;
	int i;
	int last_link;

	w->screen->set_updates(0);

	line = w->top_line;
	width = w->width - 2;
	w->screen->set_style(style);
	i = 0;

	page_data->displayed_page_link_count = 0;

	assert(page_data->lines != NULL);

	w->screen->set_style(STYLE_HELP_NORMAL);

	last_link = 0;
	style = STYLE_HELP_NORMAL;
	while (i < w->height - 2 && line < page_data->line_count)
	{
		wide = 0;
		w->screen->set_cursor(2 + w->offset_row + i, 2 + w->offset_col);

		pp = page_data->lines[line];
		assert(pp != NULL);

		while (wide < page_data->width && (pp[wide]&0xFF) != 0)
		{
			//LogInfo("%c%02X%02X\n", pp[wide] & 0xFF, pp[wide]&0xFF, pp[wide] >> 8);
			if ((pp[wide] >> 8) == HLP_F_EMPH)
			{
				style = STYLE_HELP_EMPHASIS;
				last_link = 0;
			}
			else if ((pp[wide] >> 8) == HLP_F_BOLD)
			{
				style = STYLE_HELP_BOLD;
				last_link = 0;
			}
			else if (((pp[wide] >> 8) & (HLP_F_LINK1 | HLP_F_LINK2)) != 0)
			{
				LogInfo("LastLink=%i (%c)\n", last_link, pp[wide] & 0xFF);
				style = STYLE_HELP_LINK;
				if (last_link == 0 || (last_link != ((pp[wide]>>8) & (HLP_F_LINK1 | HLP_F_LINK2))))
				{
					page_data->displayed_page_link_count += 1;
					last_link = (pp[wide]>>8) & (HLP_F_LINK1 | HLP_F_LINK2);
					//LogInfo("LastLink=%i\n", last_link);
				}

				if (page_data->displayed_page_link_count == page_data->highlight_link)
				{
					style = STYLE_HIGHLIGHT;
				}
			}
			else
			{
				style = STYLE_HELP_NORMAL;
				last_link = 0;
			}

			w->screen->set_style(style);
			display_char(w, (pp[wide] & 0xFF));

			wide++;
		}

		w->screen->set_style(STYLE_HELP_NORMAL);
		line += 1;
		i += 1;
	}

	draw_percentage(w, page_data);

	w->screen->set_updates(1);
}

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

static void FreeHelpPage(uHelpPage *p)
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


static uHelpPage* HelpReflowPage(uHelpFile *hlp, char *section, int width, int link)
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

static void FreeBreadCrumb(void *data)
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

static int GetLinkCountLine(uHelpPage *page_data, int line)
{
	int x = 0;
	int i;

	for (i=0; i < page_data->link_count; i++)
	{
		if (page_data->_links[i]->row - 1 == line)
		{
			x += 1;
		}
	}

	return x;
}

static pHelpLink ScanForLink(uHelpPage *page, int top_line)
{
	int i;
	int j;

	j = page->highlight_link;

	for (i=0; i < page->link_count; i++)
	{
		if (page->_links[i]->row >= top_line)
		{
			if (j > 0)
			{
				j -= 1;
				if (j == 0)
				{
					return page->_links[i];
				}
			}
		}
	}

	return NULL;
}

#ifdef ALFC_DATA_STRUCTURES
void help_help(uHelpFile *hdr, uWindow *w, char *page, void(*BuildWindowLayout)(uGlobalData*gd))
{
	uHelpPage *page_data;

	if (hdr != NULL)
	{
		int redraw;
		int qflag;

		qflag = 0;
		redraw = 1;

		page_data = HelpReflowPage(hdr, page, w->width - 2, -1);

		if (page_data != NULL)
		{
			int ph = 0;
			uint32_t key;

			help_draw_window(w, page_data);

			while (qflag == 0 && w->gd->screen->screen_isshutdown() == 0)
			{
				if (redraw == 1)
				{
					help_draw_page(w, page_data);
					redraw = 0;

					ph = page_data->line_count;
					ph -= (w->height - 2);
					if (ph < 0)
						ph = 0;
				}

				key = 0;
				if (w->gd->screen->screen_isresized() != 0)
				{
					int l;

					BuildWindowLayout(w->gd);

					l = page_data->highlight_link;
					FreeHelpPage(page_data);
					page_data = HelpReflowPage(hdr, page, w->width - 2, l);

					help_draw_window(w, page_data);
					help_draw_page(w, page_data);
				}
				else
				{
					key = w->screen->get_keypress();
				}

				switch (key)
				{
					case ALFC_KEY_PAGE_DOWN:
						if (ph > w->top_line)
						{
							w->top_line += w->height - 4;
							if (w->top_line > ph)
							{
								w->top_line = ph;
							}
							page_data->highlight_link = 0;
							redraw = 1;
						}
						break;

					case ALFC_KEY_PAGE_UP:
						if (w->top_line > 0)
						{
							w->top_line -= w->height - 4;
							if (w->top_line < 0)
							{
								w->top_line = 0;
							}
							page_data->highlight_link = 0;
							redraw = 1;
						}
						break;

					case ALFC_KEY_DOWN:
						if (page_data->displayed_page_link_count > page_data->highlight_link)
						{
							page_data->highlight_link += 1;
							redraw = 1;
						}
						else if (w->top_line + 1 <= ph)
						{

							page_data->highlight_link -= GetLinkCountLine(page_data, w->top_line);
							w->top_line += 1;
							redraw = 1;
						}
						break;

					case ALFC_KEY_UP:
						if ( page_data->highlight_link > 1)
						{
							page_data->highlight_link -= 1;
							redraw = 1;
						}
						else if (w->top_line > 0)
						{
							int rc;

							w->top_line -= 1;
							redraw = 1;
							rc = GetLinkCountLine(page_data, w->top_line);
							if (rc > 0)
							{
								page_data->highlight_link -= 1;
								if (page_data->highlight_link < 1)
									page_data->highlight_link = 1;
							}
						}
						break;

					case ALFC_KEY_HOME:
						{
							uHelpBreadCrumb *b;

							b = malloc(sizeof(uHelpBreadCrumb));
							b->top_line = w->top_line;
							b->highlight_line = page_data->highlight_link;
							b->prev_link = strdup(page_data->name);
							dlist_ins(hdr->lstBreadCrumbs, b);

							FreeHelpPage(page_data);
							page_data = HelpReflowPage(hdr, "Main", w->width - 2, -1);

							help_draw_window(w, page_data);
							help_draw_page(w, page_data);
							redraw = 1;
						}
						break;

					case ALFC_KEY_BACKSPACE:
					case ALFC_KEY_LEFT:
						{
							uHelpBreadCrumb *b;

							if (dlist_size(hdr->lstBreadCrumbs) > 0)
							{
								uHelpPage *px;

								dlist_remove(hdr->lstBreadCrumbs, dlist_tail(hdr->lstBreadCrumbs), (void*)&b);

								px = HelpReflowPage(hdr, b->prev_link, w->width - 2, -1);

								if (px != NULL)
								{
									px->highlight_link = b->highlight_line;
									w->top_line = b->top_line;

									FreeHelpPage(page_data);
									page_data = px;
									help_draw_window(w, page_data);
									redraw = 1;
								}
								else
								{
									status_msg(w, " FAILED TO LOAD PAGE : %s ", b->prev_link);
								}

								FreeBreadCrumb(b);
							}
						}
						break;

					case ALFC_KEY_ENTER:
					case ALFC_KEY_RIGHT:
						{
							pHelpLink link;

							link = ScanForLink(page_data, w->top_line);
							if (link == NULL)
							{
								LogError("Somehow activated an invalid link\n");
							}
							else
							{
								uHelpPage *px;
								px = HelpReflowPage(hdr, link->link, w->width - 2, -1);

								if (px != NULL)
								{
									uHelpBreadCrumb *b;

									b = malloc(sizeof(uHelpBreadCrumb));
									b->top_line = w->top_line;
									b->highlight_line = page_data->highlight_link;
									b->prev_link = strdup(page_data->name);
									dlist_ins(hdr->lstBreadCrumbs, b);

									FreeHelpPage(page_data);
									page_data = px;
									help_draw_window(w, page_data);
									redraw = 1;
								}
								else
								{
									status_msg(w, " FAILED TO LOAD PAGE : %s ", link->link);
								}
							}
						}
						break;

					case 'Q':
					case 'q':
					case ALFC_KEY_F01:
					case ALFC_KEY_ESCAPE:
					case 0x21B:	// ESC-ESC
						qflag = 1;
						break;

					default:
						break;
				}
			}
		}
		else
		{
			LogError("cound not find the requested node.\n");
		}

		if (page_data != NULL)
			FreeHelpPage(page_data);
	}
}
#endif

