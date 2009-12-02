#include "headers.h"

#include "guideheader.h"
#include "guideload.h"

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
	char *q, *oq;

	w->screen->set_style(STYLE_NORMAL);
	w->screen->window_clear(w);
	w->screen->set_style(STYLE_TITLE);
	w->screen->draw_border(w);

	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	w->screen->set_updates(0);

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
		} while (q != NULL);
		q = oq;
	}
	else
		q = page_data->name;

	w->screen->set_updates(1);

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

	w->screen->set_updates(0);
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
			else if ((pp[wide] >> 8) == HLP_F_LINK)
				style = STYLE_DIR_DOCUMENT;

			w->screen->set_style(style);
			display_char(w, (pp[wide] & 0xFF));

			wide++;
		}

		w->screen->set_style(STYLE_NORMAL);
		line += 1;
		i += 1;
	}

	w->screen->set_updates(1);
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

	if (w->width - 80 > 20)
		w->width = 80;
	else
		w->width = 60;

	if (w->height - 20 > 8)
		w->height -= 8;
	else
		w->height = 20;

	w->offset_row = (w->gd->screen->get_screen_height() - w->height) / 2;
	w->offset_col = (w->gd->screen->get_screen_width() - w->width) / 2;

	w->screen->set_style(STYLE_NORMAL);
	w->screen->window_clear(w);
	w->screen->set_style(STYLE_TITLE);
	w->screen->draw_border(w);

	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);
}

int ShowHelp(uGlobalData *gdata, char *guide, char *page)
{
	uHelpFile *hdr;
	uHelpPage *page_data;

	uWindow *orig_window;
	uWindow *w;

	orig_window = gdata->win_left;
	gdata->win_left = NULL;

	BuildWindowLayout(gdata);
	w = gdata->win_left;

	gdata->screen->set_style(STYLE_NORMAL);
	w->screen->set_cursor(2, 2);

	LogInfo("help guide=%s, page=\"%s\"\n", guide, page);

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
			LogInfo("help page %s is not found\n", page);

		if (page_data != NULL)
			FreeHelpPage(page_data);
	}
	else
		LogInfo("Could not open %s\n", guide);

	FreeHelpFile(hdr);

	gdata->win_left = orig_window;

	return 0;
}
