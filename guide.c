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
	printf("\neg:guide alfc.gd main/about/credits\n");

	exit(0);
}

static void display_char(uWindow *w, char c)
{
	char x[2];

	x[0] = c;
	x[1] = 0;
	w->screen->print((char*) &x);
}

static void draw_window(uWindow *w, uHelpPage *page_data)
{
	char *buff;
	char *q, *oq;;

	w->screen->set_style(STYLE_TITLE);
	w->screen->window_clear(w);
	w->screen->draw_border(w);
	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	assert(page_data->name != NULL);

	buff = malloc( strlen(page_data->name) + 5);

	q = strchr(page_data->name, ':');
	if(q != NULL)
	{
		do
		{
			q++;
			oq = q;
			q = strchr(q, ':');
		}while(q != NULL);
		q = oq;
	}
	else
		q = page_data->name;

	sprintf(buff, "%s ", q);
	w->screen->print(buff);
	free(buff);
}

static void draw_page(uWindow *w, uHelpPage *page_data)
{
	uint16_t *pp;
	int wide;
	int style = STYLE_NORMAL;
	int line;
	int width;
	int i;

	line = w->top_line;
	width = w->width - 2;
	w->screen->set_style(style);
	i = 0;

	assert(page_data->lines != NULL);

	w->screen->set_style(STYLE_NORMAL);

	while (i < w->height - 2 && line < page_data->line_count)
	{
		wide = 0;

		w->screen->set_cursor(2 + w->offset_row + i, 2 + w->offset_col);

		pp = page_data->lines[line];
		assert(pp != NULL);

		while (wide < page_data->width)
		{
			style = STYLE_NORMAL;
			if ((pp[wide] >> 8) == HLP_F_EMPH)
				style = STYLE_HIGHLIGHT;
			else if((pp[wide] >> 8) == HLP_F_LINK)
				style = STYLE_DIR_DOCUMENT;

			w->screen->set_style(style);
			display_char(w, (pp[wide] & 0xFF));

			wide++;
		}

		w->screen->set_style(STYLE_NORMAL);
		line += 1;
		i += 1;
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
	w->width = w->gd->screen->get_screen_width()/2;
	w->height = w->gd->screen->get_screen_height() - w->gd->screen->get_screen_height()/4;
	w->offset_col = (w->gd->screen->get_screen_width() - w->width)/ 2;
	w->offset_row = (w->gd->screen->get_screen_height() - w->height)/ 2;

	w->screen->set_style(STYLE_TITLE);
	w->screen->window_clear(w);
	w->screen->draw_border(w);

	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);
}

int ALFC_main(int start_mode, char *view_file)
{
	uHelpFile *hdr;
	uHelpPage *page_data;

	uWindow *w;
	uGlobalData *gdata;

	char *guide;
	char *page;

	char *p;
	char *q;

	ALFC_startup();
	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR | LOG_STDERR, 5000);

	if (view_file == NULL)
	{
		syntax();
	}

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

		page = strdup(p);
	}
	else
	{
		guide = strdup(q);
		page = strdup("Main");
	}

	gdata = calloc(1, sizeof(uGlobalData));

#if 10
	gdata->clr_title_bg = CLR_CYAN;
	gdata->clr_title_fg = CLR_WHITE;
	gdata->clr_background = CLR_CYAN;
	gdata->clr_foreground = CLR_BLACK;
	gdata->clr_hi_background = CLR_CYAN;
	gdata->clr_hi_foreground = CLR_BR_GREEN;

	gdata->screen = malloc(sizeof(uScreenDriver));
	memmove(gdata->screen, &screen, sizeof(uScreenDriver));

	gdata->screen->gd = gdata;
	gdata->screen->init(gdata->screen);

	gdata->screen->init_style(STYLE_DIR_DOCUMENT, CLR_BLACK, CLR_GREEN);
#else
	gdata->clr_title_bg = CLR_GREY;
	gdata->clr_title_fg = CLR_WHITE;
	gdata->clr_background = CLR_GREY;
	gdata->clr_foreground = CLR_BLACK;
	gdata->clr_hi_background = CLR_GREY;
	gdata->clr_hi_foreground = CLR_BLUE;

	gdata->screen = malloc(sizeof(uScreenDriver));
	memmove(gdata->screen, &screen, sizeof(uScreenDriver));
	gdata->screen->gd = gdata;
	gdata->screen->init(gdata->screen);

	gdata->screen->init_style(STYLE_DIR_DOCUMENT, CLR_BLACK, CLR_CYAN);
#endif

	BuildWindowLayout(gdata);

	w = gdata->win_left;

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);

	hdr = LoadHelpFile(guide);

	if (hdr != NULL)
	{
		int redraw;
		int qflag;

		qflag = 0;
		redraw = 1;

		page_data = HelpReflowPage(hdr, page, w->width - 2);
		if (page_data != NULL)
		{
			int ph;
			uint32_t key;

			draw_window(w, page_data);

			ph = page_data->line_count;
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
					BuildWindowLayout(gdata);
					FreeHelpPage(page_data);
					page_data = HelpReflowPage(hdr, page, w->width - 2);

					draw_window(w, page_data);
					draw_page(w, page_data);
				}
				else
				{
					key = w->screen->get_keypress();
				}

				switch (key)
				{
					case ALFC_KEY_DOWN:
						if (w->top_line + 1 <= ph)
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

					case 'Q':
					case 'q':
					case ALFC_KEY_F12:
					case ALFC_KEY_ESCAPE:
					case 0x21B: // ESC-ESC
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
			FreeHelpPage(page_data);
	}
	else
		w->screen->get_keypress();

	gdata->screen->deinit();

	FreeHelpFile(hdr);

	free(w);
	free(gdata->screen);
	free(gdata);

	ALFC_shutdown();

	return 0;
}
