#include "headers.h"

#include "guideheader.h"
#include "guideload.h"
#include "guidedisplay.h"


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

	if (w->width >= 60)
	{
		if (w->width >= 100)
			w->width = 80;
		else
			w->width = 60;
	}

	if (w->height <= 25)
		w->height = 20;
	else
		w->height = w->gd->screen->get_screen_height() - 14;

	w->offset_row = (w->gd->screen->get_screen_height() - w->height) / 2;
	w->offset_col = (w->gd->screen->get_screen_width() - w->width) / 2;

	w->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->window_clear(w);
	w->screen->set_style(STYLE_HELP_TITLE);
	w->screen->draw_border(w);

	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	gdata->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->set_cursor(2, 2);
}

int ShowHelp(uGlobalData *gdata, char *guide, char *page)
{
	uHelpFile *hdr;
	uWindow *w, *orig_window;

	orig_window = gdata->win_left;
	gdata->win_left = NULL;

	BuildWindowLayout(gdata);
	w = gdata->win_left;

	gdata->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->set_cursor(2, 2);

	hdr = LoadHelpFile(guide);
	if (hdr != NULL)
		help_help(hdr, w, page, BuildWindowLayout);
	else
		LogInfo("Could not open %s\n", guide);

	FreeHelpFile(hdr);

	free(gdata->win_left);

	gdata->win_left = orig_window;

	return 0;
}
