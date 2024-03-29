#ifdef DRV_NCURSES
#include "headers.h"
#include <signal.h>
#include <curses.h>
#include <ctype.h>
#include <termios.h>

static int intCurCol;
static int intCurRow;
static int intMaxHeight;
static int intMaxWidth;

static int intMaxColourPairs;
static int intStyle;
static int intUpdates;
static int intResized;

static void terminate_signal(int a);

static void setcursor(int row, int col);

struct udtStyles
{
	int style;
	int s_on;
	int s_off;
} styles[32] =
{
	{
		0, 0, 0
	},
	{
		STYLE_TITLE, A_NORMAL, A_NORMAL
	},
	{
		STYLE_NORMAL, A_NORMAL, A_NORMAL
	},
	{
		STYLE_HIGHLIGHT, A_NORMAL, A_NORMAL
	},
};

struct udtStyleColors
{
	uint32_t fg;
	uint32_t bg;
} style_colors[32];

static char* driver_name(void)
{
	return "NCURSES";
}

// convert internal colours to curses colours
static uint32_t nc_convert_colour(int color)
{
	switch (color)
	{
		case CLR_BLACK:
			return COLOR_BLACK;
			break;
		case CLR_RED:
			return COLOR_RED;
			break;
		case CLR_GREEN:
			return COLOR_GREEN;
			break;
		case CLR_BROWN:
			return COLOR_YELLOW;
			break;
		case CLR_BLUE:
			return COLOR_BLUE;
			break;
		case CLR_MAGENTA:
			return COLOR_MAGENTA;
			break;
		case CLR_CYAN:
			return COLOR_CYAN;
			break;
		case CLR_GREY:
			return COLOR_WHITE;
			break;

		case CLR_DK_GREY:
			return A_BOLD | COLOR_BLACK;
			break;
		case CLR_BR_RED:
			return A_BOLD | COLOR_RED;
			break;
		case CLR_BR_GREEN:
			return A_BOLD | COLOR_GREEN;
			break;
		case CLR_YELLOW:
			return A_BOLD | COLOR_YELLOW;
			break;
		case CLR_BR_BLUE:
			return A_BOLD | COLOR_BLUE;
			break;
		case CLR_BR_MAGENTA:
			return A_BOLD | COLOR_MAGENTA;
			break;
		case CLR_BR_CYAN:
			return A_BOLD | COLOR_CYAN;
			break;
		case CLR_WHITE:
			return A_BOLD | COLOR_WHITE;
			break;
	}

	return COLOR_WHITE;
}

static void setcolour(int bgc, int fgc)
{
	//int x;

	if (bgc > CLR_DK_GREY)
	{
		bgc -= 8;
		fgc |= A_BOLD;
	}

	attrset(COLOR_PAIR(bgc) | fgc);
}

static void dr_outchar(int s)
{
	mvaddch(intCurRow, intCurCol, s);
}

static void nc_print_hline(void)
{
	dr_outchar(ACS_HLINE);
}

static void nc_print_vline(void)
{
	dr_outchar(ACS_VLINE);
}

static void nc_draw_frame(uWindow *w)
{
	int i;

	setcursor(1 + w->offset_row, 1 + w->offset_col);
	dr_outchar(ACS_ULCORNER);
	setcursor(1 + w->offset_row, 1 + w->offset_col + (w->width - 1));
	dr_outchar(ACS_URCORNER);
	setcursor(1 + w->offset_row + (w->height - 1), 1 + w->offset_col);
	dr_outchar(ACS_LLCORNER);
	setcursor(1 + w->offset_row + (w->height - 1), 1 + w->offset_col + (w->width - 1));
	dr_outchar(ACS_LRCORNER);

	for (i = 1; i < (w->width - 1); i++)
	{
		setcursor(1 + w->offset_row, 1 + w->offset_col + i);
		nc_print_hline();
		setcursor(w->offset_row + w->height, 1 + w->offset_col + i);
		nc_print_hline();
	}

	for (i = 1; i < (w->height - 1); i++)
	{
		setcursor(1 + w->offset_row + i, 1 + w->offset_col);
		nc_print_vline();
		setcursor(1 + w->offset_row + i, w->offset_col + w->width);
		nc_print_vline();
	}
}

static void nc_clear_window(uWindow *w)
{
	int i;
	char *buff;

	buff = malloc(w->width+2);
	memset(buff, ' ', w->width+2);
	buff[w->width] = 0;

	for (i = 1; i < (w->height - 1); i++)
	{
		setcursor(1 + w->offset_row + i, 1 + w->offset_col);
		w->screen->print(buff);
	}

	free(buff);
}

static void nc_print_string(const char *s)
{
	while (*s != 0x0)
	{
		switch (*s)
		{
			case '\r':
			case '\n': // enter
				intCurCol = 0;
				intCurRow++;

				if (intCurRow >= intMaxHeight)
					intCurRow = intMaxHeight - 1;
				break;

			case 0x08: // backspace
				if (intCurCol > 0)
				{
					intCurCol--;
					dr_outchar(' ');
				}
				break;

			case 0x09: // tab
				intCurCol = ((intCurCol + 8) >> 3) << 3;

				if (intCurCol >= intMaxWidth)
					intCurCol = intMaxWidth - 1;
				break;

			default:
				dr_outchar(*s);
				intCurCol++;
				if (intCurCol > intMaxWidth)
				{
					intCurCol = 0;
					intCurRow++;
				}
				break;
		}
		s++;
	}

	if (intUpdates == 1)
	{
		doupdate();
		refresh();
	}
}

static void nc_print_string_abs(const char *s)
{
	while (*s != 0x0)
	{
		dr_outchar(*s);
		intCurCol++;
		if (intCurCol > intMaxWidth)
		{
			intCurCol = 0;
			intCurRow++;
		}

		s++;
	}

	if (intUpdates == 1)
	{
		doupdate();
		refresh();
	}
}

static uint32_t nc_get_keypress(void)
{
	uint32_t key = 0;
	int16_t ch;

	ch = wgetch(stdscr);
	if (ch == ERR)
	{
		key = 0;
	}
	else if ((ch >= 0x20 && (ch <= 0x7E) && (ch != '`')))
	{
		key = ch;
	}
#ifdef __WIN32__
	// mingw uses wgetch for getch.....
	// PDCurses has ALT_0 ...

	else if ((ch >= ALT_0) && (toupper(ch) <= ALT_Z)) // for mingw this is ALT-A to ALT-Z
	{
		// windows seems to hit here for ALT keys
		if (ch >= ALT_0 && ch <= ALT_9)
			key = ALFC_KEY_ALT + (ch - ALT_0) + '0';
		else
			key	= ALFC_KEY_ALT + (toupper(ch) - ALT_A) + 'A';
	}
#else
	else if ((ch >= 0x80) && (ch <= 0xFF))
	{
		key = ALFC_KEY_ALT + (ch - 0x80);
	}
#endif
	else if ((ch >= KEY_F0) && (ch <= KEY_F0 + 12))
	{
		key = ALFC_KEY_F01 + (ch - KEY_F0) - 1;
	}
	else if ((ch == '[') || (ch == 27))
	{
		// start of escape sequence
		ch = wgetch(stdscr);
		if(ch == 0x1B)
			key = ALFC_KEY_ESCAPE_ESCAPE;
		// SGEO: i dont like this. something funky going on with getting keypresses for home/end
		else if(ch == 0x5B)
		{
			ch = wgetch(stdscr);
			wgetch(stdscr);
			switch(ch)
			{
				case 0x31:
					key = ALFC_KEY_HOME;
					break;

				case 0x34:
					key = ALFC_KEY_END;
					break;
			}
			ch = 0;
		}
		// SGEO: i dont like this. something funky going on with getting keypresses for home/end
		else
		{
			// Linux (xterm) seems to hit here for ALT keys
			ch = toupper(ch);
			if ((ch != '[') && (ch != 0x1B)) // ALT key
				key = ALFC_KEY_ALT + ch;
			else
				ch = 0;
		}

	}
	else if (ch == '`')
	{
		/* CTRL key */
		ch = wgetch(stdscr);
		if ((ch < 256) && isalpha(ch))
		{
			ch = toupper(ch);
			key = ALFC_KEY_CTRL + toupper(ch) - 1; //((ch - 'A') + 1);
		}
		else
		{
			key = 0;
			switch (ch)
			{
#if KEY_DC != 8
				case 0x08:
					key = ALFC_KEY_CTRL + ALFC_KEY_DEL;
					break;
#endif
				case 9:
					key = ALFC_KEY_CTRL + ALFC_KEY_TAB;
					break;
				case 0x0D:
					key = ALFC_KEY_CTRL + ALFC_KEY_ENTER;
					break;
				case 0x0A:
					key = ALFC_KEY_CTRL + ALFC_KEY_ENTER;
					break;
				case 0x1B:
					key = ALFC_KEY_CTRL + ALFC_KEY_ESCAPE;
					break;

				case KEY_UP:
					key = ALFC_KEY_CTRL + ALFC_KEY_UP;
					break;
				case KEY_DOWN:
					key = ALFC_KEY_CTRL + ALFC_KEY_DOWN;
					break;
				case KEY_LEFT:
					key = ALFC_KEY_CTRL + ALFC_KEY_LEFT;
					break;
				case KEY_RIGHT:
					key = ALFC_KEY_CTRL + ALFC_KEY_RIGHT;
					break;
				case KEY_PPAGE:
					key = ALFC_KEY_CTRL + ALFC_KEY_PAGE_UP;
					break;
				case KEY_NPAGE:
					key = ALFC_KEY_CTRL + ALFC_KEY_PAGE_DOWN;
					break;
				case KEY_HOME:
					key = ALFC_KEY_CTRL + ALFC_KEY_HOME;
					break;
				case KEY_END:
					key = ALFC_KEY_CTRL + ALFC_KEY_END;
					break;
				case KEY_DC:
					key = ALFC_KEY_CTRL + ALFC_KEY_DEL;
					break;
				case KEY_SLEFT:
					key = ALFC_KEY_CTRL + ALFC_KEY_SLEFT;
					break;
				case KEY_SRIGHT:
					key = ALFC_KEY_CTRL + ALFC_KEY_SRIGHT;
					break;
				case 127:
					key = ALFC_KEY_CTRL + ALFC_KEY_DEL;
					break;
				case KEY_BACKSPACE:
					key = ALFC_KEY_CTRL + ALFC_KEY_BACKSPACE;
					break;
			}
		}
	}
	else
	{
		switch (ch)
		{
#if KEY_DC != 8
#ifdef CTL_BKSP
			case CTL_BKSP:
				key = ALFC_KEY_CTRL + ALFC_KEY_BACKSPACE;
				break;
#endif
			case 0x08:
				key = ALFC_KEY_DEL;
				break;
#endif
#ifdef CTL_TAB
			case CTL_TAB:
				key = ALFC_KEY_CTRL + ALFC_KEY_TAB;
				break;
#endif

			case 9:
				key = ALFC_KEY_TAB;
				break;
#ifdef CTL_ENTER
			case CTL_ENTER:
				key = ALFC_KEY_CTRL + ALFC_KEY_ENTER;
				break;
#endif

			case 0x0D:
			case 0x0A:
				key = ALFC_KEY_ENTER;
				break;

			case 0x1B:
				key = ALFC_KEY_ESCAPE;
				break;
#ifdef CTL_UP
			case CTL_UP:
				key = ALFC_KEY_CTRL + ALFC_KEY_UP;
				break;
#endif

			case KEY_UP:
				key = ALFC_KEY_UP;
				break;
#ifdef CTL_DOWN
			case CTL_DOWN:
				key = ALFC_KEY_CTRL + ALFC_KEY_DOWN;
				break;
#endif

			case KEY_DOWN:
				key = ALFC_KEY_DOWN;
				break;
#ifdef CTL_LEFT
			case CTL_LEFT:
				key = ALFC_KEY_CTRL + ALFC_KEY_LEFT;
				break;
#endif

			case KEY_LEFT:
				key = ALFC_KEY_LEFT;
				break;

#ifdef CTL_RIGHT
			case CTL_RIGHT:
				key = ALFC_KEY_CTRL + ALFC_KEY_RIGHT;
				break;
#endif

			case KEY_RIGHT:
				key = ALFC_KEY_RIGHT;
				break;

#ifdef CTL_PGUP
			case CTL_PGUP:
				key = ALFC_KEY_CTRL + ALFC_KEY_PAGE_UP;
				break;
#endif

			case KEY_PPAGE:
				key = ALFC_KEY_PAGE_UP;
				break;

#ifdef CTL_PGDN
			case CTL_PGDN:
				key = ALFC_KEY_CTRL + ALFC_KEY_PAGE_DOWN;
				break;
#endif

			case KEY_NPAGE:
				key = ALFC_KEY_PAGE_DOWN;
				break;

#ifdef CTL_HOME
			case CTL_HOME:
				key = ALFC_KEY_CTRL + ALFC_KEY_HOME;
				break;
#endif

			case KEY_HOME:
				key = ALFC_KEY_HOME;
				break;

#ifdef CTL_END
			case CTL_END:
				key = ALFC_KEY_CTRL + ALFC_KEY_END;
				break;
#endif

			case KEY_END:
				key = ALFC_KEY_END;
				break;

			case KEY_DC:
				key = ALFC_KEY_DEL;
				break;

			case KEY_SLEFT:
				key = ALFC_KEY_SLEFT;
				break;

			case KEY_SRIGHT:
				key = ALFC_KEY_SRIGHT;
				break;

			case 127:
				key = ALFC_KEY_DEL;
				break;

			case KEY_BACKSPACE:
				// SGEO : 20101217 - remove CTRL from here...
				key = ALFC_KEY_BACKSPACE;
				break;

			default:
				if ((ch > 0) && (ch <= 26))
					key = ALFC_KEY_CTRL + ch + 'A' - 1;	// CTRL keys
				break;
		}
	}
	return key;
}

static void init_style(int style, uint32_t fg, uint32_t bg)
{
	if (style >= 1 && style <= MAX_STYLES)
	{
		styles[style].s_off = A_NORMAL;
		if ((nc_convert_colour(bg) & A_BOLD) == A_BOLD || (nc_convert_colour(fg) & A_BOLD) == A_BOLD)
		{
			styles[style].s_off = A_BOLD;
			styles[style].s_on = A_BOLD;
		}
		else
			styles[style].s_on = A_NORMAL;
	}

	init_pair(style, nc_convert_colour(fg) & 0xFF, nc_convert_colour(bg) & 0xFF);
}

static void nc_init_style(int style, uint32_t fg, uint32_t bg)
{
	if (bg == -1)
		bg = style_colors[style].bg;

	style_colors[style].fg = fg;
	style_colors[style].bg = bg;
	init_style(style, fg, bg);
}


static void init_list_styles(uScreenDriver *scr)
{
	init_style(STYLE_VIEW_EDIT_EOL, CLR_BR_GREEN, CLR_BLACK); // end of line marker in viewer...
}

static void init_dir_styles(uScreenDriver *scr)
{
	init_style(STYLE_DIR_EXEC, style_colors[STYLE_DIR_EXEC].fg, style_colors[STYLE_DIR_EXEC].bg);
	init_style(STYLE_DIR_LINK, style_colors[STYLE_DIR_LINK].fg, style_colors[STYLE_DIR_LINK].bg);
	init_style(STYLE_DIR_IMAGE, style_colors[STYLE_DIR_IMAGE].fg, style_colors[STYLE_DIR_IMAGE].bg);
	init_style(STYLE_DIR_DIR, style_colors[STYLE_DIR_DIR].fg, style_colors[STYLE_DIR_DIR].bg);
	init_style(STYLE_DIR_DOCUMENT, style_colors[STYLE_DIR_DOCUMENT].fg, style_colors[STYLE_DIR_DOCUMENT].bg);
	init_style(STYLE_DIR_ARCHIVE, style_colors[STYLE_DIR_ARCHIVE].fg, style_colors[STYLE_DIR_ARCHIVE].bg);
	init_style(STYLE_DIR_BACKUP, style_colors[STYLE_DIR_BACKUP].fg, style_colors[STYLE_DIR_BACKUP].bg);
	init_style(STYLE_DIR_MEDIA, style_colors[STYLE_DIR_MEDIA].fg, style_colors[STYLE_DIR_MEDIA].bg);
}

static void init_dir_styles_masterlist(uScreenDriver *scr)
{
	style_colors[STYLE_DIR_EXEC].fg = CLR_BR_GREEN;
	style_colors[STYLE_DIR_EXEC].bg = CLR_BLACK;

	style_colors[STYLE_DIR_LINK].fg = CLR_YELLOW;
	style_colors[STYLE_DIR_LINK].bg = CLR_BLACK;

	style_colors[STYLE_DIR_IMAGE].fg = CLR_BR_BLUE;
	style_colors[STYLE_DIR_IMAGE].bg = CLR_BLACK;

	style_colors[STYLE_DIR_EXEC].fg = CLR_BR_GREEN;
	style_colors[STYLE_DIR_EXEC].bg = CLR_BLACK;

	style_colors[STYLE_DIR_DIR].fg = CLR_BR_BLUE;
	style_colors[STYLE_DIR_DIR].bg = CLR_BLACK;

	style_colors[STYLE_DIR_DOCUMENT].fg = CLR_BLUE;
	style_colors[STYLE_DIR_DOCUMENT].bg = CLR_BLACK;

	style_colors[STYLE_DIR_ARCHIVE].fg = CLR_BR_RED;
	style_colors[STYLE_DIR_ARCHIVE].bg = CLR_BLACK;

	style_colors[STYLE_DIR_BACKUP].fg = CLR_DK_GREY;
	style_colors[STYLE_DIR_BACKUP].bg = CLR_BLACK;

	style_colors[STYLE_DIR_MEDIA].fg = CLR_BR_MAGENTA;
	style_colors[STYLE_DIR_MEDIA].bg = CLR_BLACK;

	init_dir_styles(scr);
}


static void init_view_styles(uScreenDriver *scr)
{
	init_style(STYLE_VIEW_EDIT_EOL, CLR_BR_GREEN, CLR_BLACK); // end of line marker in viewer...
}

static int nc_screen_init(uScreenDriver *scr)
{
	int i;

	signal(SIGKILL, terminate_signal);

#ifndef __WIN32__
	//signal(SIGKILL, terminate_signal); /*setting SIGKILL signal handler*/
	signal(SIGQUIT, terminate_signal); /*setting SIGQUIT signal handler*/
	signal(SIGSEGV, terminate_signal); /*setting SIGSEGV signal handler*/
#endif


/*
	tcgetattr(STDIN_FILENO, &tattr);
	tcgetattr(STDIN_FILENO, &tattr_bak);
	tattr.c_lflag &= ~(ICANON|ECHO);
	tattr.c_cc[VMIN]=1;
	tattr.c_cc[VTIME]=0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
*/

	// initialise ncurses
	initscr();

	if (has_colors() == TRUE)
		start_color();

	noecho();
	nonl();
	//cbreak();

	keypad(stdscr, TRUE);	// gimme the keypad
	raw(); 					// raw
	meta(stdscr, TRUE);		// + meta

	intMaxHeight = scr->get_screen_height();
	intMaxWidth = scr->get_screen_width();

	// init some colour pairs
	for (i = 1; i > 0; i++)
	{
		if (init_pair((char) i, CLR_WHITE, CLR_BLACK) == ERR)
			break;
	}

	intMaxColourPairs = i - 1;
	//LogInfo("Can handle %i colourpairs\n", intMaxColourPairs);

	init_pair(CLR_BLACK, COLOR_BLACK, COLOR_BLACK);
	init_pair(CLR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(CLR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(CLR_BROWN, COLOR_YELLOW, COLOR_BLACK);
	init_pair(CLR_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(CLR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CLR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(CLR_GREY, COLOR_WHITE, COLOR_BLACK);

	// init to black
	init_style(STYLE_TITLE, CLR_CYAN, CLR_BLACK);
	init_style(STYLE_NORMAL, CLR_GREY, CLR_BLACK);
	init_style(STYLE_HIGHLIGHT, CLR_YELLOW, CLR_BLACK);	// highlight line
	scr->set_style(STYLE_NORMAL);
	//scr->cls();

	init_style(STYLE_TITLE, scr->gd->clr_title_fg, scr->gd->clr_title_bg); // title bar
	init_style(STYLE_NORMAL, scr->gd->clr_foreground, scr->gd->clr_background);	// default
	init_style(STYLE_HIGHLIGHT, scr->gd->clr_hi_foreground, scr->gd->clr_hi_background); // highlight line

	init_dir_styles_masterlist(scr);

	scr->set_style(STYLE_NORMAL);

	scr->enable_raw();

	return 0;
}

static void erase_eol(void)
{
	int i;

	i = intCurCol;

	for (; intCurCol < intMaxWidth; intCurCol++)
		addch(' ');

	intCurCol = i;
}

static void setcursor(int row, int col)
{
	intCurCol = (col - 1);
	intCurRow = (row - 1);

	move(intCurRow, intCurCol);
	if (intUpdates == 1)
	{
		doupdate();
	}
}

static int nc_screen_deinit(void)
{
	setcursor(intMaxHeight, 1);

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	setcolour(1, A_NORMAL);

#ifdef PDCURSES
	//reset_screen();
	//curs_set(i);
#endif


	noraw();
	endwin();

	return 0;
}

static int nc_get_screen_height(void)
{
	return LINES;
}

static int nc_get_screen_width(void)
{
	return COLS;
}

static void nc_cls(void)
{
	int i;

	for (i = 0; i < intMaxHeight; i++)
	{
		setcursor(1 + i, 1);
		erase_eol();
	}

	intCurCol = 0;
	intCurRow = 0;
}

static void nc_set_style(int style)
{
	intStyle = style;
	setcolour(style, styles[style].s_on);
}

static void terminate_signal(int a)
{
	nc_screen_deinit();
	exit(-1);
}

static int nc_resized(void)
{
	int rc = intResized;
	intResized = 0;
	return rc;
}

static int nc_isshutdown(void)
{
	return 0;
}

static void nc_set_updates(int set)
{
	if (set == 0)
		intUpdates = 0;
	else
		intUpdates = 1;

	if (intUpdates == 1)
	{
		refresh();
		doupdate();
	}
}

static int nc_get_updates(void)
{
	return intUpdates;
}


static void nc_updatewindow(void)
{
	doupdate();
}

static void nc_trigger_redraw(void)
{
	intResized = 1;
}

// need this for our exec
static void nc_going_exec(void)
{
	noraw();
	endwin();
}

struct termios orig_termios;
static void disable_raw(void)
{
#ifdef BUILD_UNIXLIKE
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
#endif
}

static void enable_raw(void)
{
#ifdef BUILD_UNIXLIKE
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(endwin);
	atexit(noraw);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

uScreenDriver screen =
{
	NULL,

	driver_name,

	nc_screen_init,	// init screen
	nc_screen_deinit, // uninit
	nc_cls,	// clear scren
	nc_get_screen_height,
	nc_get_screen_width,
	nc_get_keypress,
	nc_print_string,
	nc_print_string_abs,
	nc_set_style,
	setcursor,
	erase_eol,
	nc_draw_frame,
	nc_clear_window,
	nc_init_style,
	nc_print_hline,
	nc_print_vline,
	nc_set_updates,
	nc_get_updates,

	init_dir_styles,
	init_view_styles,
	init_list_styles,
	nc_resized,
	nc_isshutdown,
	nc_updatewindow,
	nc_trigger_redraw,
	nc_going_exec,

	enable_raw,
	disable_raw
};
#endif
