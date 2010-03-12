#include "headers.h"
#include <zlib.h>

#include "guideheader.h"
#include "guideload.h"
#include "guidedisplay.h"

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
	if (line <= 0)
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

						// alias I/i for Index, H/h for home.
					case 'i':
					case 'I':
					case 'h':
					case 'H':
					case ALFC_KEY_HOME:
						{
							uHelpBreadCrumb *b;

							b = calloc(1, sizeof(uHelpBreadCrumb));
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
							if (dlist_size(hdr->lstBreadCrumbs) > 0)
							{
								uHelpPage *px;
								uHelpBreadCrumb *b = NULL;

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
									//FreeBreadCrumb(b);
								}
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

									b = calloc(1, sizeof(uHelpBreadCrumb));
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
