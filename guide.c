#include "headers.h"

#include "guideheader.h"
#include "guideload.h"

// foobar for the x11 interface...
char *start_left;
char *start_right;

char* gstr_WindowTitle = "HypterText Help Guide Viewer";
extern char* gstr_WindowTitle;

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

static void syntax(void)
{
	printf("guide guidename title/index/page\n");
	printf("\neg:gdump alfc.gd main/about/credits\n");

	exit(0);
}

static void display_char(uWindow *w, char c)
{
	char x[2];

	x[0] = c;
	x[1] = 0;
	w->screen->print((char*) &x);
}

static void draw_window(uWindow *w, uIM_GuidePage *page_data)
{
	DLElement *e;
	char *pp;
	uIM_GuideLine *line;

	w->screen->set_style(STYLE_NORMAL);
	w->screen->cls();
	w->screen->set_style(STYLE_TITLE);
	w->screen->draw_border(w);
	w->screen->set_cursor(1, 2);
	w->screen->print(" Guide Reader : ");

	e = dlist_head(page_data->lstLines);

	if (e != NULL)
	{
		char *buff;

		line = dlist_data(e);
		pp = (char*) line->text;
		e = dlist_next(e);

		w->screen->set_style(STYLE_TITLE);
		w->screen->set_cursor(1, 17);

		buff = malloc(strlen(pp) + 4);
		sprintf(buff, " %s ", pp);
		w->screen->print(buff);
		free(buff);
	}
}

static void draw_page(uWindow *w, uIM_GuidePage *page_data)
{
	DLElement *e;
	char *pp;
	int i;
	int fixed;
	int wide;
	int style = STYLE_NORMAL;
	uIM_GuideLine *line;

	char *start_line;

	start_line = NULL;

	e = dlist_head(page_data->lstLines);
	e = dlist_next(e);

	// skip lines to the correct start line.
	wide = w->top_line;
	while (wide > 0)
	{
		int ll;

		line = dlist_data(e);
		ll = line->length;

		if (ll < w->width - 2)
		{
			wide -= 1;
			e = dlist_next(e);
		}
		else
		{
			start_line = (char*) line->text;

			while (wide > 0 && ll > w->width - 2)
			{
				ll -= w->width - 2;
				start_line += w->width - 2;
				wide -= 1;
			}

			if (wide > 0 && start_line != (char*) line->text)
			{
				wide -= 1;
				e = dlist_next(e);
				start_line = NULL;
			}
		}
	}

	fixed = 0;

	line = dlist_data(e);
	if ((line->flags & 0xFF) == eLF_Bold)
		style = STYLE_DIR_LINK;
	if ((line->flags & 0xFF) == eLF_Reverse)
		style = STYLE_DIR_ARCHIVE;
	if ((line->flags & 0xFF) == eLF_Underline)
		style = STYLE_DIR_DOCUMENT;
	if ((line->flags & 0xFF) == eLF_Fixed)
		style = STYLE_DIR_DIR;

	fixed = (line->flags >> 8) & 0xFF;

	w->screen->set_style(STYLE_NORMAL);
	i = 0;
	while (e != NULL && i < w->height - 2)
	{
		line = dlist_data(e);

		if (start_line != NULL)
			pp = start_line;
		else
			pp = (char*) line->text;

		start_line = NULL;

		wide = 0;

		w->screen->set_cursor(2 + w->offset_row + i, 2 + w->offset_col);
		w->screen->set_style(style);

		while (*pp != 0 && wide < w->width - 2)
		{
			int wlen;
			int rlen;
			int ffixed;


			// calculate word length.
			wlen = 0;
			rlen = 0;
			ffixed = fixed;
			while (pp[wlen] != 0x0 && pp[wlen] != 0x0A && pp[wlen] != ' ')
			{
				switch (pp[wlen])
				{
					case '{':
					case '}':
						if (fixed == 0)
						{
							if (memcmp(pp + wlen, "{{{", 3) == 0)
							{
								ffixed += 1;
								wlen += 3;
							}
							else if (memcmp(pp + wlen, "}}}", 3) == 0)
							{
								ffixed -= 1;
								wlen += 3;
							}
							else
							{
								wlen++;
								rlen++;
							}
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
						if (ffixed == 1)
							rlen++;
						wlen++;
						break;

					default:
						wlen++;
						rlen++;
						break;
				}
			}

			// draw it on screen...
			if (wide + rlen > w->width - 2)
			{
				i += 1;
				wide = 0;
				w->screen->set_cursor(2 + w->offset_row + i, 2 + w->offset_col);
			}

			switch (*pp)
			{
				// link
				case '%':
					if (fixed == 0)
					{
						if (style == STYLE_DIR_EXEC)
							style = STYLE_NORMAL;
						else
							style = STYLE_DIR_EXEC;
						pp++;
					}
					else
					{
						display_char(w, *pp++);
						wide++;
					}
					break;

				case '{':
					if (memcmp(pp, "{{{", 3) == 0)
					{
						fixed += 1;
						if (fixed == 1)
						{
							pp += 3;
							style = STYLE_DIR_DIR;
						}
						else
						{
							display_char(w, *pp++);
							wide++;
							display_char(w, *pp++);
							wide++;
							display_char(w, *pp++);
							wide++;
						}
					}
					break;

				case '}':
					if (memcmp(pp, "}}}", 3) == 0)
					{
						fixed -= 1;
						if (fixed == 0)
						{
							pp += 3;
							fixed = 0;
							style = STYLE_NORMAL;
						}
						else
						{
							display_char(w, *pp++);
							wide++;
							display_char(w, *pp++);
							wide++;
							display_char(w, *pp++);
							wide++;
						}
					}
					break;

				case '~':
					if (fixed == 0)
					{
						if (style == STYLE_DIR_LINK)
							style = STYLE_NORMAL;
						else
							style = STYLE_DIR_LINK;
						pp++;
					}
					else
					{
						display_char(w, *pp++);
						wide++;
					}
					break;

				case '^':
					if (fixed == 0)
					{
						if (style == STYLE_DIR_ARCHIVE)
							style = STYLE_NORMAL;
						else
							style = STYLE_DIR_ARCHIVE;
						pp++;
					}
					else
					{
						display_char(w, *pp++);
						wide++;
					}
					break;

				case '@':
					if (fixed == 0)
					{
						if (style == STYLE_DIR_DOCUMENT)
							style = STYLE_NORMAL;
						else
							style = STYLE_DIR_DOCUMENT;
						pp++;
					}
					else
					{
						display_char(w, *pp++);
						wide++;
					}
					break;

				default:
					display_char(w, *pp++);
					wide++;
					break;
			}

			w->screen->set_style(style);
		}

		w->screen->set_style(STYLE_NORMAL);
		while (wide < w->width - 2)
		{
			wide += 1;
			display_char(w, ' ');
		}

		i += 1;
		e = dlist_next(e);
	}
}

static void BuildWindowLayout(uGlobalData *gdata)
{
	uWindow *w;

	if (gdata->win_left == NULL)
	{
		w = calloc(1, sizeof(uWindow));
		w->gd = gdata;
		w->screen = gdata->screen;
		gdata->win_left = w;
	}

	w = gdata->win_left;

	w->offset_row = 0;
	w->offset_col = 0;
	w->width = w->gd->screen->get_screen_width();
	w->height = w->gd->screen->get_screen_height();
	w->top_line = 0;
	w->highlight_line = 0;

	w->screen->set_style(STYLE_TITLE);
	w->screen->draw_border(w);
	w->screen->set_cursor(1, 2);
	w->screen->print(" Guide Reader : ");

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);
}

int ALFC_main(int start_mode, char *view_file)
{
	uIM_GuideHeader *hdr;

	int count;
	char **page;
	char *p, *q;
	uIM_GuidePage *page_data;

	uWindow *w;
	uGlobalData *gdata;

	char *guide;

	ALFC_startup();

	if (view_file == NULL)
	{
		syntax();
	}

	page = calloc(32, sizeof(char*));

	count = 0;

	p = view_file;
	while (*p == ' ' && *p != 0x0)
		p++;

	q = p;
	p = strchr(q, ' ');
	if (p == NULL && *q == 0)
	{
		fprintf(stderr, "Missing guide name\n");
		exit(0);
	}

	if (p != NULL)
	{
		*p = 0;
		guide = strdup(q);
		*p = ' ';

		while (*p == ' ')
			p++;

		q = p;
		p = strchr(p, '/');
		if (p == NULL)
		{
			page[count++] = strdup(q);
		}
		else
		{
			while (p != NULL && *p != 0x0)
			{
				*p = 0;

				page[count++] = strdup(q);
				*p = '/';
				q = p + 1;
				p = strchr(q, '/');

				if (p == NULL)
				{
					page[count++] = strdup(q);
				}
			}
		}
	}
	else
	{
		guide = strdup(q);
	}

	gdata = calloc(1, sizeof(uGlobalData));

	gdata->clr_title_bg = CLR_BLUE;
	gdata->clr_title_fg = CLR_WHITE;
	gdata->clr_background = CLR_BLACK;
	gdata->clr_foreground = CLR_GREY;
	gdata->clr_hi_background = CLR_RED;
	gdata->clr_hi_foreground = CLR_YELLOW;

	gdata->screen = malloc(sizeof(uScreenDriver));
	memmove(gdata->screen, &screen, sizeof(uScreenDriver));

	gdata->screen->gd = gdata;
	gdata->screen->init(gdata->screen);

	gdata->screen->set_style(STYLE_NORMAL);
	gdata->screen->cls();

	BuildWindowLayout(gdata);
	w = gdata->win_left;

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);
	hdr = LoadGuide(guide);
	if (hdr != NULL)
	{
		int redraw;
		int qflag;


		// setup for first node in list as default viewing node.
		if (count == 0)
		{
			DLElement *e;
			uIM_Node *n;

			e = dlist_head(hdr->lstNodes);
			if (e != NULL)
			{
				int i;

				n = dlist_data(e);
				count = n->node_count;
				for (i = 0; i < count; i++)
					page[i] = strdup(n->nodes[i]);
			}
		}

		redraw = 1;
		qflag = 0;

		page_data = ReflowPage(hdr, count, page);
		if (page_data != NULL)
		{
			int ph;
			uint32_t key;

			draw_window(w, page_data);

			ph = dlist_size(page_data->lstLines) - 1;
			ph -= (w->height - 2);
			if (ph < 0)
				ph = 0;

			while (qflag == 0 && gdata->screen->screen_isshutdown() == 0)
			{
				if (redraw == 1)
				{
					draw_page(w, page_data);
					redraw = 0;
				}

				key = 0;
				if (gdata->screen->screen_isresized() != 0)
				{
					gdata->screen->cls();
					BuildWindowLayout(gdata);
					w = gdata->win_left;

					draw_window(w, page_data);
					draw_page(w, page_data);
					redraw = 0;
					continue;
				}
				else
				{
					key = w->screen->get_keypress();
				}

				switch (key)
				{
					case ALFC_KEY_DOWN:
						if (w->top_line + 1 < ph)
						{
							w->top_line += 1;
							redraw = 1;
						}
						break;

					case ALFC_KEY_UP:
						if (w->top_line > 0)
						{
							w->top_line -= 1;
							redraw = 1;
						}
						break;

					case ALFC_KEY_ESCAPE:
						qflag = 1;
						break;

				}
			}
		}
		else
		{
			fprintf(stderr, "cound not find the requested node.\n");
		}

		if (page_data != NULL)
			FreePage(page_data);

		FreeGuide(hdr);
	}

	while (count > 0)
		free(page[--count]);

	free(page);

	if (guide != NULL)
		free(guide);

	free(w);
	free(gdata->screen);
	free(gdata);

	ALFC_shutdown();

	return 0;
}
