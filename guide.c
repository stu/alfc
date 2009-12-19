#include "headers.h"

#include "guideheader.h"
#include "guideload.h"

// foobar for the x11 interface...
char *start_left;
char *start_right;

char* gstr_WindowTitle = "HypterText Help Guide Viewer";
extern char* gstr_WindowTitle;

static void syntax(void)
{
	printf("guide guidename title/index/page\n");
	printf("\neg:guide alfc.gd main/about/credits\n");

	exit(0);
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

	w->screen->set_style(STYLE_HELP_TITLE);
	w->screen->window_clear(w);
	w->screen->draw_border(w);

	w->screen->set_cursor(w->offset_row + 1, w->offset_col + 2);
	w->screen->print(" Guide Reader : ");

	gdata->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->set_cursor(2, 2);
}

int ALFC_main(int start_mode, char *view_file)
{
	uHelpFile *hdr;
	uWindow *w;
	uGlobalData *gdata;

	char *guide;
	char *page;

	char *p;
	char *q;

	ALFC_startup();
	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR | LOG_STDERR, 5000);
	LogInfo("help guide loader - Stu George -- v%i.%02i/%04i - Built on " __DATE__ "; " __TIME__ "\n", VersionMajor(), VersionMinor(), VersionBuild());

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

	gdata->clr_title_bg = CLR_CYAN;
	gdata->clr_title_fg = CLR_WHITE;
	gdata->clr_background = CLR_CYAN;
	gdata->clr_foreground = CLR_BLACK;
	gdata->clr_hi_background = CLR_BLUE;
	gdata->clr_hi_foreground = CLR_YELLOW;

	gdata->screen = malloc(sizeof(uScreenDriver));
	memmove(gdata->screen, &screen, sizeof(uScreenDriver));

	gdata->screen->gd = gdata;
	gdata->screen->init(gdata->screen);

	gdata->screen->init_style(STYLE_HELP_TITLE, CLR_WHITE, CLR_CYAN);
	gdata->screen->init_style(STYLE_HELP_NORMAL, CLR_BLACK, CLR_CYAN);
	gdata->screen->init_style(STYLE_HELP_BOLD, CLR_BLUE, CLR_CYAN);
	gdata->screen->init_style(STYLE_HELP_EMPHASIS, CLR_WHITE, CLR_CYAN);
	gdata->screen->init_style(STYLE_HELP_LINK, CLR_YELLOW, CLR_CYAN);


	BuildWindowLayout(gdata);

	w = gdata->win_left;

	gdata->screen->set_style(STYLE_HELP_NORMAL);
	w->screen->set_cursor(2, 2);

	hdr = LoadHelpFile(guide);

	if (hdr != NULL)
		help_help(hdr, w, page, BuildWindowLayout);

	free(page);
	free(guide);

	gdata->screen->deinit();

	FreeHelpFile(hdr);

	free(w);
	free(gdata->screen);
	free(gdata);

	ALFC_shutdown();
	LogWrite_Shutdown();

	return 0;
}
