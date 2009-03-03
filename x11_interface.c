#include "headers.h"
#include "rl/rllib.h"

#ifdef __MINGW_H
#include <windows.h>
#else
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#endif

#include "x11_fonts.h"

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
} styles[16] =
{
	{ 0,0,0, 0, 0},
	{ STYLE_TITLE, 0, 0, 0, 0},
	{ STYLE_NORMAL, 0, 0, 0, 0 },
	{ STYLE_HIGHLIGHT, 0, 0, 0, 0 },

};


rgbcolor xc_colors[32];

static void GetScreenDimensions(int *w, int *h)
{
	*w = window_width_in_pixels();
	*h = window_height_in_pixels();
}

static char* driver_name(void)
{
	return "X11";
}

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
	x11_set_colour(CLR_GREY, 0xAA, 0xAA, 0xAA);
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
	display_char(s&0xFF, xc_colors[styles[intStyle].fg], xc_colors[styles[intStyle].bg], intCurCol, intCurRow);
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

static int x11_resized(void)
{
	return window_resized();
}

static uint32_t x11_get_keypress(void)
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

static void x11_init_style(int style, uint32_t fg, uint32_t bg)
{
	init_style(style, fg, bg);
}

static void init_dir_styles(uScreenDriver *scr)
{
	init_style(STYLE_DIR_EXEC, 		CLR_BR_GREEN, CLR_BLACK);				// exec
	init_style(STYLE_DIR_LINK, 		CLR_YELLOW, CLR_BLACK);					// link

	init_style(STYLE_DIR_IMAGE, 	CLR_BR_BLUE, CLR_BLACK);				// image
	init_style(STYLE_DIR_DIR, 		CLR_BR_BLUE, CLR_BLACK);				// dir
	init_style(STYLE_DIR_DOCUMENT, 	CLR_BR_BLUE, CLR_BLACK);				// document
	init_style(STYLE_DIR_ARCHIVE, 	CLR_BR_BLUE, CLR_BLACK);				// archive
	init_style(STYLE_DIR_BACKUP, 	CLR_DK_GREY, CLR_BLACK);				// archive
}

static void init_view_styles(uScreenDriver *scr)
{
	init_style(STYLE_VIEW_EDIT_EOL, CLR_BR_GREEN, CLR_BLACK);	// end of line marker in viewer...
}

static int x11_screen_init(uScreenDriver *scr)
{
	int w, h;

	GetScreenDimensions(&w, &h);

	if(w == 0 || h == 0)
	{
		w = 1024;
		h = 768;
	}

	if( w < 100*10 || h < 25*20)
		//create_window(w/8 - 4, h/12 - 5, "Another Linux File Commander", x11_data_font_small, x11_data_font_small_SIZE);
		create_window(100, 25, "Another Linux File Commander", x11_data_font_small, x11_data_font_small_SIZE);
	else
		//create_window(w/10 - 4,  h/20 - 5, "Another Linux File Commander", x11_data_font, x11_data_font_SIZE);
		create_window(100, 25, "Another Linux File Commander", x11_data_font, x11_data_font_SIZE);

	x11_init_colours();

	init_style(STYLE_NORMAL,	CLR_GREY, 		CLR_BLACK);
	init_style(STYLE_TITLE,  	CLR_WHITE, 		CLR_RED);
	init_style(STYLE_HIGHLIGHT, CLR_GREY, 		CLR_BLUE);

	init_style(STYLE_TITLE, scr->gd->clr_title_fg, scr->gd->clr_title_bg);					// title bar
	init_style(STYLE_NORMAL, scr->gd->clr_foreground, scr->gd->clr_background);				// default
	init_style(STYLE_HIGHLIGHT, scr->gd->clr_hi_foreground, scr->gd->clr_hi_background);	// highlight line

	init_dir_styles(scr);

	scr->set_style(STYLE_NORMAL);
	x11_cls();
	//x11_print_string("Another Linux File Commander");

	update_window();

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

	clear();

	for(i=0; i < window_height_in_chars(); i++)
	{
		x11_setcursor(1+i, 1);
		x11_erase_eol();
	}

	x11_setcursor(1,1);
	update_window();
}

static void x11_set_style(int style)
{
	intStyle = style;
}


int rlmain(int argc, char *argv[])
{
	int i;
	int start_mode = eMode_Directory;
	char *view_file = NULL;

	for(i=1; i<argc; i++)
	{
		if(argv[i][0] == '-')
		{
			if(1+i < argc && strcmp("-l", argv[i]) == 0)
			{
				start_left = argv[i+1];
			}
			if(1+i < argc && strcmp("-r", argv[i]) == 0)
			{
				start_right = argv[i+1];
			}
			else if(strcmp("-?", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
			{
				fprintf(stderr, "\n-v\t\tVersion\n"
								"-?\t\tHelp\n"
								"-l DIR\t\tStart left side in directory DIR\n"
								"-r DIR\t\tStart right side in directory DIR\n"
								"-view FILE\tStart in viewer mode\n"
								);
				exit(0);
			}
			else if(strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0)
			{
				fprintf(stderr, "ALFC v%i.%02i/%04i\n", VersionMajor(), VersionMinor(), VersionBuild());
				exit(0);
			}
			else if(strcmp("-view", argv[i]) == 0 && 1+i < argc)
			{
				view_file = argv[1+i];
				i+=1;
				start_mode = eMode_Viewer;
			}
		}
	}

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

	init_dir_styles,
	init_view_styles,
	x11_resized
};
