#ifdef DRV_GUI
#include "headers.h"
#include "guicore.h"

#ifdef __WIN32__
#include <windows.h>
#else
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif

#include "gui_fonts.h"

static int intCurCol;
static int intCurRow;
static int intStyle;
static int intUpdates;
static int intResized;

static void gui_cls(void);
static void gui_setcursor(int row, int col);

static int intStyleMode;

static struct udtStyles
{
	int style;
	int s_on;
	int s_off;
	int fg;
	int bg;
} styles[16] =
{
	{ 0,0,0, 0, 0},
	{ STYLE_TITLE, 0, 0, 0, 0},
	{ STYLE_NORMAL, 0, 0, 0, 0 },
	{ STYLE_HIGHLIGHT, 0, 0, 0, 0 },

};

static int style_data_dir[][3] = {
		{ STYLE_DIR_EXEC, 		CLR_BR_GREEN, 	CLR_BLACK },
		{ STYLE_DIR_LINK, 		CLR_YELLOW, 	CLR_BLACK },
		{ STYLE_DIR_IMAGE, 		CLR_BR_CYAN, 	CLR_BLACK },
		{ STYLE_DIR_DIR, 		CLR_BR_BLUE, 	CLR_BLACK },
		{ STYLE_DIR_DOCUMENT, 	CLR_MAGENTA, 	CLR_BLACK },
		{ STYLE_DIR_ARCHIVE, 	CLR_RED, 		CLR_BLACK },
		{ STYLE_DIR_BACKUP, 	CLR_DK_GREY,	CLR_BLACK },

		{ -1, -1, -1 }
};

static int style_data_view[][3] = {
		{ STYLE_VIEW_EDIT_EOL, 		CLR_BR_GREEN, 	CLR_BLACK },
		{ -1, -1, -1 }
};

static int style_data_base[][3] = {
		{ STYLE_TITLE, 			CLR_WHITE, 	CLR_BLUE },
		{ STYLE_NORMAL, 		CLR_GREY, 	CLR_BLACK },
		{ STYLE_HIGHLIGHT, 		CLR_WHITE, 	CLR_RED },

		{ -1, -1, -1 }
};

rgbcolor xc_colors[32];

static void GetScreenDimensions(int *w, int *h)
{
	*w = window_width_in_pixels();
	*h = window_height_in_pixels();
}

static char* driver_name(void)
{
	return "GUI";
}

static int convert_colour(int c)
{
	return c - CLR_BLACK;
}


static void gui_set_colour(int c, int r, int g, int b)
{
	xc_colors[convert_colour(c)] = rgb(r, g, b);
}

static void gui_init_colours(void)
{
	gui_set_colour(CLR_BLACK, 0, 0, 0);
	gui_set_colour(CLR_GREY, 0xAA, 0xAA, 0xAA);
	gui_set_colour(CLR_RED, 0x80, 0, 0);
	gui_set_colour(CLR_GREEN, 0, 0x80, 0);
	gui_set_colour(CLR_BROWN, 0xAE, 0xB3, 00);
	gui_set_colour(CLR_BLUE, 0x00, 0, 0x80);
	gui_set_colour(CLR_MAGENTA, 0x80, 0x00, 0x80);
	gui_set_colour(CLR_CYAN, 0x00, 0x80, 0x80);
	gui_set_colour(CLR_DK_GREY, 0x40, 0x40, 0x40);
	gui_set_colour(CLR_BR_RED, 0xff, 0x40, 0x4);
	gui_set_colour(CLR_BR_GREEN, 0, 0xff, 0);
	gui_set_colour(CLR_YELLOW, 0xff, 0xff, 0);
	gui_set_colour(CLR_BR_BLUE, 0x40, 0x40, 0xff);
	gui_set_colour(CLR_BR_MAGENTA, 0xFF, 0x00, 0xFF);
	gui_set_colour(CLR_BR_CYAN, 00, 0xFF, 0xFF);
	gui_set_colour(CLR_WHITE, 0xFF, 0xFF, 0xFF);
}


static void dr_outchar(int s)
{
	display_char(s&0xFF, xc_colors[styles[intStyle].fg], xc_colors[styles[intStyle].bg], intCurCol, intCurRow);
}

static void gui_print_hline(void)
{
	dr_outchar(196);
}

static void gui_print_vline(void)
{
	dr_outchar(179);
}


static void gui_clear_window(uWindow *w)
{
	int i;
	char *buff;

	buff = malloc(w->width+2);
	memset(buff, ' ', w->width+2);
	buff[w->width] = 0;

	for (i = 1; i < (w->height - 1); i++)
	{
		gui_setcursor(1 + w->offset_row + i, 1 + w->offset_col);
		w->screen->print(buff);
	}

	free(buff);
}

static void gui_draw_frame(uWindow *w)
{
	int i;

	gui_setcursor(1 + w->offset_row, 1 + w->offset_col);										dr_outchar(218);
	gui_setcursor(1 + w->offset_row, 1 + w->offset_col + (w->width-1));						dr_outchar(191);
	gui_setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col);						dr_outchar(192);
	gui_setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col + (w->width-1));		dr_outchar(217);

	for(i = 1 ; i < (w->width-1); i++)
	{
		gui_setcursor(1 + w->offset_row, 1 + w->offset_col + i);			gui_print_hline();
		gui_setcursor(w->offset_row + w->height, 1 + w->offset_col + i );	gui_print_hline();
	}

	for(i = 1; i < (w->height-1); i++)
	{
		gui_setcursor(1 + w->offset_row + i , 1 + w->offset_col);			gui_print_vline();
		gui_setcursor(1 + w->offset_row + i, w->offset_col + w->width);	gui_print_vline();
	}
}

static void gui_print_string(const char *s)
{
	while(*s!=0x0)
	{
		switch(*s)
		{
			case '\r':
			case '\n':							// enter
		 		intCurCol=0;
		 		intCurRow++;

				if(intCurRow >= window_height_in_chars())
					intCurRow = window_height_in_chars()-1;
				break;

			case 0x08:							// backspace
				if(intCurCol>0)
				{
					intCurCol--;
					dr_outchar(' ');
				}
				break;

			case 0x09:							// tab
				intCurCol = ((intCurCol+8)>>3)<<3;

				if(intCurCol >= window_width_in_chars())
					intCurCol=window_width_in_chars()-1;
				break;

			default:
				dr_outchar(*s);
				intCurCol++;
				if(intCurCol>window_width_in_chars())
				{
					intCurCol=0;
					intCurRow++;
				}
				break;
		}
		s++;
	}
}

static void gui_print_string_abs(const char *s)
{
	while(*s!=0x0)
	{
		dr_outchar(*s);
		intCurCol++;
		if(intCurCol>window_width_in_chars())
		{
			intCurCol=0;
			intCurRow++;
		}

 		s++;
	}
}

static int gui_isshutdown(void)
{
	return window_isshutdown();
}

static int gui_resized(void)
{
	int rc = window_resized() | intResized;
	intResized = 0;

	return rc;
}

static void gui_trigger_redraw(void)
{
	intResized = 1;
}

static uint32_t gui_get_keypress(void)
{
	key k;
	uint32_t x = 0;

	k = get_key();

	if((k.ctrl & 1) == 1)
		x = ALFC_KEY_CTRL + k.c + 'A' - 1;
	else if((k.ctrl & 2) == 2)
		x = ALFC_KEY_ALT + toupper(k.c);
	else
	{
		if(k.c >= RLKEY_START_CODE)
		{
			switch(k.c)
			{
				case RLKEY_F1: x = ALFC_KEY_F01; break;
				case RLKEY_F2: x = ALFC_KEY_F02; break;
				case RLKEY_F3: x = ALFC_KEY_F03; break;
				case RLKEY_F4: x = ALFC_KEY_F04; break;
				case RLKEY_F5: x = ALFC_KEY_F05; break;
				case RLKEY_F6: x = ALFC_KEY_F06; break;
				case RLKEY_F7: x = ALFC_KEY_F07; break;
				case RLKEY_F8: x = ALFC_KEY_F08; break;
				case RLKEY_F9: x = ALFC_KEY_F09; break;
				case RLKEY_F10: x = ALFC_KEY_F10; break;
				case RLKEY_F11: x = ALFC_KEY_F11; break;
				case RLKEY_F12: x = ALFC_KEY_F12; break;
				case RLKEY_LEFT: x = ALFC_KEY_LEFT; break;
				case RLKEY_RIGHT: x = ALFC_KEY_RIGHT; break;
				case RLKEY_UP: x = ALFC_KEY_UP; break;
				case RLKEY_DOWN: x = ALFC_KEY_DOWN; break;
				case RLKEY_INSERT: x = ALFC_KEY_INS; break;
				case RLKEY_DELETE: x = ALFC_KEY_DEL; break;
				case RLKEY_HOME: x = ALFC_KEY_HOME; break;
				case RLKEY_END: x = ALFC_KEY_END; break;
				case RLKEY_PRIOR: x = ALFC_KEY_PAGE_UP; break;
				case RLKEY_NEXT: x = ALFC_KEY_PAGE_DOWN; break;
				case RLKEY_BACK: x = ALFC_KEY_BACKSPACE; break;
			}
		}
		else
		{
			switch(k.c)
			{
				case 0x08: x = ALFC_KEY_DEL; break;
				case 0x09: x = ALFC_KEY_TAB; break;
				case 0x0D: x = ALFC_KEY_ENTER; break;
				case 0x1B: x = ALFC_KEY_ESCAPE; break;

				default:
					x = k.c;
					break;
			}
		}
	}

	return x;
}

static void init_style(int style, uint32_t fg, uint32_t bg)
{
	styles[style].fg = convert_colour(fg);
	styles[style].bg = convert_colour(bg);
}

static void gui_update_style(int style, uint32_t fg, uint32_t bg)
{
	int i;

	if(style < STYLE_00 && style > 0)
	{
		for(i=0; style_data_base[i][0] != -1; i++)
		{
			if(style_data_base[i][0] == style)
			{
				style_data_base[i][1] = fg;
				style_data_base[i][2] = bg;
				init_style(style, fg, bg);
				return;
			}
		}
	}

	if(intStyleMode == 0)
	{
		for(i=0; style_data_dir[i][0] != -1; i++)
		{
			if(style_data_dir[i][0] == style)
			{
				style_data_dir[i][1] = fg;
				style_data_dir[i][2] = bg;
				init_style(style, fg, bg);
				return;
			}
		}
	}
	else if(intStyleMode == 1)
	{
		for(i=0; style_data_view[i][0] != -1; i++)
		{
			if(style_data_view[i][0] == style)
			{
				style_data_view[i][1] = fg;
				style_data_view[i][2] = bg;
				init_style(style, fg, bg);
				return;
			}
		}
	}
}

static void init_dir_styles(uScreenDriver *scr)
{
	int i;

	intStyleMode = 0;
	for(i=0; style_data_dir[i][0] != -1; i++)
	{
		init_style(style_data_dir[i][0], style_data_dir[i][1], style_data_dir[i][2]);
	}
}

static void init_view_styles(uScreenDriver *scr)
{
	int i;

	intStyleMode = 1;
	for(i=0; style_data_dir[i][0] != -1; i++)
	{
		init_style(style_data_view[i][0], style_data_view[i][1], style_data_view[i][2]);
	}
}

static void init_base_styles(uScreenDriver *scr)
{
	int i;

	for(i=0; style_data_base[i][0] != -1; i++)
	{
		init_style(style_data_base[i][0], style_data_base[i][1], style_data_base[i][2]);
	}
}


static int gui_screen_init(uScreenDriver *scr)
{
	int w, h;

	GetScreenDimensions(&w, &h);

	if(w == 0 || h == 0)
	{
		w = 1024;
		h = 768;
	}

	if( w < 100*10 || h < 25*20)
		//create_window(w/8 - 4, h/12 - 5, "Another Linux File Commander", gui_data_font_small, gui_data_font_small_SIZE);
		create_window(100, 25, gstr_WindowTitle, gui_data_font_small, gui_data_font_small_SIZE);
	else
		//create_window(w/10 - 4,  h/20 - 5, "Another Linux File Commander", gui_data_font, gui_data_font_SIZE);
		create_window(100, 25, gstr_WindowTitle, gui_data_font, gui_data_font_SIZE);

	gui_init_colours();

	init_base_styles(scr);
	init_dir_styles(scr);

	// init to black
	init_style(STYLE_TITLE, CLR_GREY, CLR_BLACK);
	scr->set_style(STYLE_NORMAL);
	//scr->cls();

	init_style(STYLE_TITLE, scr->gd->clr_title_fg, scr->gd->clr_title_bg);					// title bar
	init_style(STYLE_NORMAL, scr->gd->clr_foreground, scr->gd->clr_background);				// default
	init_style(STYLE_HIGHLIGHT, scr->gd->clr_hi_foreground, scr->gd->clr_hi_background);	// highlight line

	scr->set_style(STYLE_NORMAL);

	return 0;
}

static void gui_erase_eol(void)
{
	int i;

	for(i=intCurCol; i < window_width_in_chars(); i++)
	{
		dr_outchar(' ');
		intCurCol += 1;
	}
}


static void gui_setcursor(int row, int col)
{
	intCurCol = (col-1);
	intCurRow = (row-1);
}

static int gui_screen_deinit(void)
{
	destroy_window();
	return 0;
}

static int gui_get_screen_height(void)
{
	return window_height_in_chars();
}

static int gui_get_screen_width(void)
{
	return window_width_in_chars();
}

static void gui_cls(void)
{
	int i;

	//clear();

	for(i=0; i < window_height_in_chars(); i++)
	{
		gui_setcursor(1+i, 1);
		gui_erase_eol();
	}

	gui_setcursor(1,1);
	update_window();
}

static void gui_set_style(int style)
{
	intStyle = style;
}

static void gui_set_updates(int set)
{
	if(set == 0)
		intUpdates = 0;
	else
		intUpdates = 1;
}

static int gui_get_updates(void)
{
	return intUpdates;
}

static void gui_update_window(void)
{
	update_window();
}

static void gui_going_exec(void)
{
	//
}

uScreenDriver screen =
{
	NULL,

	driver_name,

	gui_screen_init,		// init screen
	gui_screen_deinit,	// uninit
	gui_cls,				// clear scren
	gui_get_screen_height,
	gui_get_screen_width,
	gui_get_keypress,
	gui_print_string,
	gui_print_string_abs,
	gui_set_style,
	gui_setcursor,
	gui_erase_eol,
	gui_draw_frame,
	gui_clear_window,
	gui_update_style,
	gui_print_hline,
	gui_print_vline,
	gui_set_updates,
	gui_get_updates,

	init_dir_styles,
	init_view_styles,
	gui_resized,
	gui_isshutdown,
	gui_update_window,
	gui_trigger_redraw,
	gui_going_exec
};
#endif
