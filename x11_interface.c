#include "headers.h"
#include "rl/rllib.h"

#ifdef __MINGW_H
#include <windows.h>
#else
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif


static int intCurCol;
static int intCurRow;
static int intStyle;

static void x11_cls(void);
static void x11_setcursor(int row, int col);


struct udtStyles
{
	int style;
	int s_on;
	int s_off;
	int fg;
	int bg;
} styles[1 + MAX_STYLES] =
{
	{ 0,0,0, 0, 0},
	{ STYLE_TITLE, 0, 0, 0, 0},
	{ STYLE_NORMAL, 0, 0, 0, 0 },
	{ STYLE_HIGHLIGHT, 0, 0, 0, 0 },
	{ STYLE_EDIT_EOL, 0, 0, 0, 0 }

};

static char* driver_name(void)
{
	return "x11";
}


rgbcolor xc_colors[32];


static int convert_colour(int c)
{
	return c - CLR_BLACK;
}


static void x11_set_colour(int c, int r, int g, int b)
{
	xc_colors[convert_colour(c)] = rgb(r, g, b);
}

static void x11_init_colours(void)
{
	x11_set_colour(CLR_BLACK, 0, 0, 0);
	x11_set_colour(CLR_GREY, 0xc0, 0xc0, 0xc0);
	x11_set_colour(CLR_RED, 0x80, 0, 0);
	x11_set_colour(CLR_GREEN, 0, 0x80, 0);
	x11_set_colour(CLR_BROWN, 0xAE, 0xB3, 00);
	x11_set_colour(CLR_BLUE, 0x00, 0, 0x80);
	x11_set_colour(CLR_MAGENTA, 0x80, 0x00, 0x80);
	x11_set_colour(CLR_CYAN, 0x00, 0x80, 0x80);
	x11_set_colour(CLR_DK_GREY, 0x40, 0x40, 0x40);
	x11_set_colour(CLR_BR_RED, 0xff, 0x40, 0x4);
	x11_set_colour(CLR_BR_GREEN, 0, 0xff, 0);
	x11_set_colour(CLR_YELLOW, 0xff, 0xff, 0);
	x11_set_colour(CLR_BR_BLUE, 0x40, 0x40, 0xff);
	x11_set_colour(CLR_BR_MAGENTA, 0xFF, 0x00, 0xFF);
	x11_set_colour(CLR_BR_CYAN, 00, 0xFF, 0xFF);
	x11_set_colour(CLR_WHITE, 0xFF, 0xFF, 0xFF);
}


static void dr_outchar(int s)
{
	display_char(s, xc_colors[styles[intStyle].fg], xc_colors[styles[intStyle].bg], intCurCol, intCurRow);
}

static void x11_print_hline(void)
{
	dr_outchar(196);
}

static void x11_print_vline(void)
{
	dr_outchar(179);
}

static void x11_draw_frame(uWindow *w)
{
	int i;

	x11_setcursor(1 + w->offset_row, 1 + w->offset_col);										dr_outchar(218);
	x11_setcursor(1 + w->offset_row, 1 + w->offset_col + (w->width-1));						dr_outchar(191);
	x11_setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col);						dr_outchar(192);
	x11_setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col + (w->width-1));		dr_outchar(217);

	for(i = 1 ; i < (w->width-1); i++)
	{
		x11_setcursor(1 + w->offset_row, 1 + w->offset_col + i);			x11_print_hline();
		x11_setcursor(w->offset_row + w->height, 1 + w->offset_col + i );	x11_print_hline();
	}

	for(i = 1; i < (w->height-1); i++)
	{
		x11_setcursor(1 + w->offset_row + i , 1 + w->offset_col);			x11_print_vline();
		x11_setcursor(1 + w->offset_row + i, w->offset_col + w->width);	x11_print_vline();
	}
}

static void x11_print_string(const char *s)
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

static void x11_print_string_abs(const char *s)
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

static uint32_t x11_get_keypress(void)
{
	key k;
	uint32_t x = 0;

	k = get_key();

	//LogInfo("key = %04X, %04X, %02X\n", k.ctrl, k.shift, k.c);

	if((k.ctrl & 1) == 1)
		x = ALFC_KEY_CTRL + k.c + 'A' - 1;
	else if((k.ctrl & 2) == 2)
		x = ALFC_KEY_ALT + toupper(k.c);
	else
	{
		if(k.c >= 0x800000)
		{
			switch(k.c - 0x800000)
			{
#ifdef __MINGW_H
				case VK_F1: x = ALFC_KEY_F01; break;
				case VK_F2: x = ALFC_KEY_F02; break;
				case VK_F3: x = ALFC_KEY_F03; break;
				case VK_F4: x = ALFC_KEY_F04; break;
				case VK_F5: x = ALFC_KEY_F05; break;
				case VK_F6: x = ALFC_KEY_F06; break;
				case VK_F7: x = ALFC_KEY_F07; break;
				case VK_F8: x = ALFC_KEY_F08; break;
				case VK_F9: x = ALFC_KEY_F09; break;
				case VK_F10: x = ALFC_KEY_F10; break;
				case VK_F11: x = ALFC_KEY_F11; break;
				case VK_F12: x = ALFC_KEY_F12; break;
				case VK_LEFT: x = ALFC_KEY_LEFT; break;
				case VK_RIGHT: x = ALFC_KEY_RIGHT; break;
				case VK_UP: x = ALFC_KEY_UP; break;
				case VK_DOWN: x = ALFC_KEY_DOWN; break;
				case VK_INSERT: x = ALFC_KEY_INS; break;
				case VK_DELETE: x = ALFC_KEY_DEL; break;
				case VK_HOME: x = ALFC_KEY_HOME; break;
				case VK_END: x = ALFC_KEY_END; break;
				case VK_PRIOR: x = ALFC_KEY_PAGE_UP; break;
				case VK_NEXT: x = ALFC_KEY_PAGE_DOWN; break;
				case VK_BACK: x = ALFC_KEY_BACKSPACE; break;
#else
				case XK_F1: x = ALFC_KEY_F01; break;
				case XK_F2: x = ALFC_KEY_F02; break;
				case XK_F3: x = ALFC_KEY_F03; break;
				case XK_F4: x = ALFC_KEY_F04; break;
				case XK_F5: x = ALFC_KEY_F05; break;
				case XK_F6: x = ALFC_KEY_F06; break;
				case XK_F7: x = ALFC_KEY_F07; break;
				case XK_F8: x = ALFC_KEY_F08; break;
				case XK_F9: x = ALFC_KEY_F09; break;
				case XK_F10: x = ALFC_KEY_F10; break;
				case XK_F11: x = ALFC_KEY_F11; break;
				case XK_F12: x = ALFC_KEY_F12; break;
				case XK_Left: x = ALFC_KEY_LEFT; break;
				case XK_Right: x = ALFC_KEY_RIGHT; break;
				case XK_Up: x = ALFC_KEY_UP; break;
				case XK_Down: x = ALFC_KEY_DOWN; break;
				case XK_Insert: x = ALFC_KEY_INS; break;
				case XK_Delete: x = ALFC_KEY_DEL; break;
				case XK_Home: x = ALFC_KEY_HOME; break;
				case XK_End: x = ALFC_KEY_END; break;
				case XK_Page_Up: x = ALFC_KEY_PAGE_UP; break;
				case XK_Page_Down: x = ALFC_KEY_PAGE_DOWN; break;
				case XK_BackSpace: x = ALFC_KEY_BACKSPACE; break;
#endif
			}
		}
		else
		{
			switch(k.c)
			{
				case 0x08: x = ALFC_KEY_DEL; break;
				case 0x09: x = ALFC_KEY_TAB; break;
				case 0x0D: x = ALFC_KEY_ENTER; break;

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

static void x11_init_style(int style, uint32_t fg, uint32_t bg)
{
	init_style(style, fg, bg);
}

static int x11_screen_init(uScreenDriver *scr)
{
	int w, h;

	ALFC_GetScreenDimensions(&w, &h);

	LogInfo("Screen dimensions %ix%i\n", w, h);

	if(w == 0 || h == 0)
	{
		w = 1600;
		h = 1000;
	}

	if( w < 100*10 || h < 40*20)
		create_window(w/8 - 4, h/12 - 5, "Another Linux File Commander", "font_small.rlf");
	else
		create_window(w/10 - 4,  h/20 - 5, "Another Linux File Commander", "font.rlf");

	x11_init_colours();

	init_style(STYLE_NORMAL,	CLR_GREY, 		CLR_BLACK);
	init_style(STYLE_TITLE,  	CLR_WHITE, 		CLR_RED);
	init_style(STYLE_HIGHLIGHT, CLR_GREY, 		CLR_BLUE);
	init_style(STYLE_EDIT_EOL, 	CLR_BR_BLUE, 	CLR_BLACK);

	init_style(STYLE_TITLE, scr->gd->clr_title_fg, scr->gd->clr_title_bg);					// title bar
	init_style(STYLE_NORMAL, scr->gd->clr_foreground, scr->gd->clr_background);				// default
	init_style(STYLE_HIGHLIGHT, scr->gd->clr_hi_foreground, scr->gd->clr_hi_background);	// highlight line

	init_style(STYLE_EDIT_EOL, CLR_BR_GREEN, scr->gd->clr_background);	// highlight line

	scr->set_style(STYLE_NORMAL);
	scr->cls();

	x11_cls();
	x11_print_string("Another Linux File Commander");

	//x11_get_keypress();

	return 0;
}

static void x11_erase_eol(void)
{
	int i;

	for(i=intCurCol; i < window_width_in_chars(); i++)
	{
		dr_outchar(' ');
		intCurCol += 1;
	}
}


static void x11_setcursor(int row, int col)
{
	intCurCol = (col-1);
	intCurRow = (row-1);
}

static int x11_screen_deinit(void)
{
	destroy_window();
	return 0;
}

static int x11_get_screen_height(void)
{
	return window_height_in_chars();
}

static int x11_get_screen_width(void)
{
	return window_width_in_chars();
}

static void x11_cls(void)
{
	int i;

	for(i=0; i < window_height_in_chars(); i++)
	{
		x11_setcursor(1+i, 1);
		x11_erase_eol();
	}

	x11_setcursor(1,1);
}

static void x11_set_style(int style)
{
	switch(style)
	{
		case STYLE_NORMAL:
		case STYLE_TITLE:
		case STYLE_HIGHLIGHT:
		case STYLE_EDIT_EOL:
			intStyle = style;
			break;
	}
}


int rlmain()
{
	return ALFC_main(eMode_Directory, NULL);
}

uScreenDriver screen =
{
	NULL,

	driver_name,

	x11_screen_init,		// init screen
	x11_screen_deinit,	// uninit
	x11_cls,				// clear scren
	x11_get_screen_height,
	x11_get_screen_width,
	x11_get_keypress,
	x11_print_string,
	x11_print_string_abs,
	x11_set_style,
	x11_setcursor,
	x11_erase_eol,
	x11_draw_frame,
	x11_init_style,
	x11_print_hline,
	x11_print_vline,
};
