#include "headers.h"
#ifndef __MINGW_H
#include <signal.h>
#endif
#include <curses.h>
#include <ctype.h>

static int intCurCol;
static int intCurRow;
static int intMaxHeight;
static int intMaxWidth;

static int intMaxColourPairs;
static int intStyle;

#ifndef __MINGW_H
static void terminate_signal(int a);
#endif

static void setcursor(int row, int col);


struct udtStyles
{
	int style;
	int s_on;
	int s_off;
} styles[1 + MAX_STYLES] =
{
	{ 0,0,0},
	{ STYLE_TITLE, A_NORMAL, A_NORMAL },
	{ STYLE_NORMAL, A_NORMAL, A_NORMAL },
	{ STYLE_HIGHLIGHT, A_NORMAL, A_NORMAL },
	{ STYLE_EDIT_EOL, A_NORMAL, A_NORMAL }

};

// convert internal colours to curses colours
static uint32_t nc_convert_colour(int color)
{
	switch(color)
	{
		case CLR_BLACK: return COLOR_BLACK; break;
		case CLR_RED: return COLOR_RED; break;
		case CLR_GREEN: return COLOR_GREEN; break;
		case CLR_BROWN: return COLOR_YELLOW; break;
		case CLR_BLUE: return COLOR_BLUE; break;
		case CLR_MAGENTA: return COLOR_MAGENTA; break;
		case CLR_CYAN: return COLOR_CYAN; break;
		case CLR_GREY: return COLOR_WHITE; break;

		case CLR_DK_GREY: return A_BOLD | COLOR_BLACK; break;
		case CLR_BR_RED: return A_BOLD | COLOR_RED; break;
		case CLR_BR_GREEN: return A_BOLD | COLOR_GREEN; break;
		case CLR_YELLOW: return A_BOLD | COLOR_YELLOW; break;
		case CLR_BR_BLUE: return A_BOLD | COLOR_BLUE; break;
		case CLR_BR_MAGENTA: return A_BOLD | COLOR_MAGENTA; break;
		case CLR_BR_CYAN: return A_BOLD | COLOR_CYAN; break;
		case CLR_WHITE: return A_BOLD | COLOR_WHITE; break;
	}

	return COLOR_WHITE;
}

static void setcolour(int bgc, int fgc)
{
	if(bgc > CLR_DK_GREY)
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

	setcolour(STYLE_TITLE, styles[STYLE_TITLE].s_on);

	setcursor(1 + w->offset_row, 1 + w->offset_col);					dr_outchar(ACS_ULCORNER);
	setcursor(1 + w->offset_row, 1 + w->offset_col + (w->width-1));				dr_outchar(ACS_URCORNER);
	setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col);			dr_outchar(ACS_LLCORNER);
	setcursor(1 + w->offset_row + (w->height-1), 1 + w->offset_col + (w->width-1));		dr_outchar(ACS_LRCORNER);

	for(i = 1 ; i < (w->width-1); i++)
	{
		setcursor(1 + w->offset_row, 1 + w->offset_col + i);					nc_print_hline();
		setcursor(w->offset_row + w->height, 1 + w->offset_col + i );			nc_print_hline();
	}

	for(i = 1; i < (w->height-1); i++)
	{
		setcursor(1 + w->offset_row + i , 1 + w->offset_col);		nc_print_vline();
		setcursor(1 + w->offset_row + i, w->offset_col + w->width);			nc_print_vline();
	}

	setcolour(STYLE_NORMAL, styles[STYLE_NORMAL].s_on);
}

static void nc_print_string(const char *s)
{
	while(*s!=0x0)
	{
		switch(*s)
		{
			case '\r':
			case '\n':							// enter
		 		intCurCol=0;
		 		intCurRow++;

				if(intCurRow >= intMaxHeight)
					intCurRow = intMaxHeight-1;
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

				if(intCurCol >= intMaxWidth)
					intCurCol=intMaxWidth-1;
				break;

			default:
				dr_outchar(*s);
				intCurCol++;
				if(intCurCol>intMaxWidth)
				{
					intCurCol=0;
					intCurRow++;
				}
				break;
		}
		s++;
	}
	doupdate();
	refresh();
}

static void nc_print_string_abs(const char *s)
{
	while(*s!=0x0)
	{
		dr_outchar(*s);
		intCurCol++;
		if(intCurCol>intMaxWidth)
		{
			intCurCol=0;
			intCurRow++;
		}

 		s++;
	}
	doupdate();
	refresh();
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
	else if ((ch >= 0x20 && (ch <= 0x7E ) && (ch != '`')))
	{
		key = ch;
	}
#ifdef __MINGW_H
	// mingw uses wgetch for getch.....
	// PDCurses has ALT_0 ...
	else if ((ch >= ALT_0) && (toupper(ch) <= ALT_Z)) // for mingw this is ALT-A to ALT-Z
	{
		// windows seems to hit here for ALT keys
		if(ch >= ALT_0 && ch <= ALT_9)
			key = ALFC_KEY_ALT + (ch - ALT_0) + '0';
		else
			key = ALFC_KEY_ALT + (toupper(ch) - ALT_A) + 'A';
	}
#else
	else if ((ch >= 0x80) && (ch <= 0xFF))
	{
		key = ALFC_KEY_ALT + (ch - 0x80);
	}
#endif
	else if ((ch >= KEY_F0) && (ch <= KEY_F0 + 12))
	{
		key = ALFC_KEY_F00 + (ch - KEY_F0);
	}
	else if ((ch == '[') || (ch == 27))
	{  /* start of escape sequence */
		ch = getch();

		// Linux (xterm) seems to hit here for ALT keys
		ch = toupper(ch);
		if ((ch != '[') && (ch != 0x27))  /* ALT key */
			key = ALFC_KEY_ALT + ch;
		else
			ch = 0;
	}
	else if (ch == '`')
	{  /* CTRL key */
		ch = getch();
		if ((ch < 256) && isalpha(ch))
		{
			ch = toupper(ch);
			key = ALFC_KEY_CTRL + ((ch - 'A') + 1);
		}
		else
			key = 0;
	}
	else
	{
		switch(ch)
		{
			case 0x0D:				key = ALFC_KEY_ENTER;		break;
			case 0x0A:				key = ALFC_KEY_ENTER;		break;
			case KEY_UP:			key = ALFC_KEY_UP;			break;
			case KEY_DOWN:			key = ALFC_KEY_DOWN;		break;
			case KEY_LEFT:			key = ALFC_KEY_LEFT;		break;
			case KEY_RIGHT:			key = ALFC_KEY_RIGHT;		break;
			case KEY_PPAGE:			key = ALFC_KEY_PAGE_UP;		break;
			case KEY_NPAGE:			key = ALFC_KEY_PAGE_DOWN;	break;
			case KEY_HOME:			key = ALFC_KEY_HOME;		break;
			case KEY_END:			key = ALFC_KEY_END;			break;
			case KEY_DC:			key = ALFC_KEY_DEL;			break;
			case KEY_SLEFT:			key = ALFC_KEY_SLEFT;		break;
			case KEY_SRIGHT:		key = ALFC_KEY_SRIGHT;		break;
			case 127:				key = ALFC_KEY_DEL;			break;
#if KEY_DC != 8
			case 0x08:				key = ALFC_KEY_DEL;			break;
#endif
			case KEY_BACKSPACE:		key = ALFC_KEY_BACKSPACE;	break;
			case 9:					key = ALFC_KEY_TAB;    		break;
			default:
			if ((ch > 0) && (ch <= 26))
				key = ALFC_KEY_CTRL + ch; // CTRL keys
		}
	}

	return key;
}

static void init_style(int style, uint32_t fg, uint32_t bg)
{
	if(style >= 1 && style <= MAX_STYLES)
	{
		styles[style].s_off = A_NORMAL;
		if( (nc_convert_colour(bg) & A_BOLD) == A_BOLD || (nc_convert_colour(fg) & A_BOLD) == A_BOLD)
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
	init_style(style, fg, bg);
}

static int nc_screen_init(uScreenDriver *scr)
{
	int i;

#ifndef __MINGW_H
	signal(SIGKILL, terminate_signal ); /*setting SIGKILL signal handler*/
	signal(SIGQUIT, terminate_signal ); /*setting SIGQUIT signal handler*/
	signal(SIGSEGV, terminate_signal ); /*setting SIGSEGV signal handler*/
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

	if(has_colors() == TRUE)
		start_color();

	noecho();
	nonl();
	cbreak();

	keypad(stdscr, TRUE);		// gimme the keypad
	raw();						// fork me raw
	meta(stdscr, TRUE);			// fork me meta

	intMaxHeight = scr->get_screen_height();
	intMaxWidth = scr->get_screen_width();

	// init some colour pairs
	for(i=1; i > 0; i++)
	{
		if(init_pair((char)i, CLR_WHITE, CLR_BLACK)==ERR)
			break;
	}

	intMaxColourPairs = i - 1;
	//LogInfo("Can handle %i colourpairs\n", intMaxColourPairs);

	init_pair(CLR_BLACK,	COLOR_BLACK,	COLOR_BLACK);
	init_pair(CLR_RED,		COLOR_RED,		COLOR_BLACK);
	init_pair(CLR_GREEN,	COLOR_GREEN,	COLOR_BLACK);
	init_pair(CLR_BROWN,	COLOR_YELLOW,	COLOR_BLACK);
	init_pair(CLR_BLUE,		COLOR_BLUE,		COLOR_BLACK);
	init_pair(CLR_MAGENTA,	COLOR_MAGENTA,	COLOR_BLACK);
	init_pair(CLR_CYAN,		COLOR_CYAN,		COLOR_BLACK);
	init_pair(CLR_GREY,		COLOR_WHITE,	COLOR_BLACK);

	init_style(STYLE_TITLE, scr->gd->clr_title_fg, scr->gd->clr_title_bg);					// title bar
	init_style(STYLE_NORMAL, scr->gd->clr_foreground, scr->gd->clr_background);				// default
	init_style(STYLE_HIGHLIGHT, scr->gd->clr_hi_foreground, scr->gd->clr_hi_background);	// highlight line

	init_style(STYLE_EDIT_EOL, CLR_BR_GREEN, scr->gd->clr_background);	// highlight line

	scr->set_style(STYLE_NORMAL);
	scr->cls();

	//LogInfo("Screen is %ix%i\n", LINES, COLS);

	return 0;
}

static  void erase_eol(void)
{
	int i;

	i=intCurCol;

	for( ; intCurCol < intMaxWidth ; intCurCol++)
		addch(' ');

	intCurCol=i;
}


static void setcursor(int row, int col)
{
	intCurCol = (col-1);
	intCurRow = (row-1);

	move(intCurRow, intCurCol);
	doupdate();
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

	for(i=0; i< intMaxHeight; i++)
	{
		setcursor(1+i, 1);
		erase_eol();
	}

	intCurCol=0;
	intCurRow=0;
}

static void nc_set_style(int style)
{
	switch(style)
	{
		case STYLE_NORMAL:
		case STYLE_TITLE:
		case STYLE_HIGHLIGHT:
		case STYLE_EDIT_EOL:
			intStyle = style;
			setcolour(style, styles[style].s_on);
			break;
	}
}

#ifndef __MINGW_H
static void terminate_signal(int a)
{
	nc_screen_deinit();
	exit(-1);
}
#endif

uScreenDriver screen_ncurses =
{
	NULL,

	nc_screen_init,		// init screen
	nc_screen_deinit,	// uninit
	nc_cls,				// clear scren
	nc_get_screen_height,
	nc_get_screen_width,
	nc_get_keypress,
	nc_print_string,
	nc_print_string_abs,
	nc_set_style,
	setcursor,
	erase_eol,
	nc_draw_frame,
	nc_init_style,
	nc_print_hline,
	nc_print_vline,
};
