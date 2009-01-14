#include "headers.h"

static int intCurCol;
static int intCurRow;
static int intMaxHeight;
static int intMaxWidth;

static int intMaxColourPairs;
static int intStyle;

#define FRM_UL	0
#define FRM_U	1
#define FRM_UR	2
#define FRM_L	3
#define FRM_R	4
#define FRM_LL	5
#define FRM_B	6
#define FRM_LR	7

static void setcursor(int row, int col);

#define MAX_STYLES	3
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
	{ STYLE_HIGHLIGHT, A_NORMAL, A_NORMAL }
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
		setcursor(1 + w->offset_row, 1 + w->offset_col + i);					dr_outchar(ACS_HLINE);
		setcursor(w->offset_row + w->height, 1 + w->offset_col + i );			dr_outchar(ACS_HLINE);
	}

	for(i = 1; i < (w->height-1); i++)
	{
		setcursor(1 + w->offset_row + i , 1 + w->offset_col);		dr_outchar(ACS_VLINE);
		setcursor(1 + w->offset_row + i, w->offset_col + w->width);			dr_outchar(ACS_VLINE);
	}

	setcolour(STYLE_NORMAL, styles[STYLE_NORMAL].s_on);
}

/*
static void draw_frame(void)
{
	uWindow w;

	w.width = COLS;
	w.height = LINES;
	w.offset_row = 0;
	w.offset_col = 0;

	nc_draw_frame(&w);
}
*/

static void nc_print_string(const char *s)
{
	while(*s!=0x0)
	{
		switch(*s)
		{
			case '}':
				setcolour(intStyle, styles[intStyle].s_off);
				break;
			case '{':
				setcolour(intStyle, styles[intStyle].s_on);
				break;

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
}

static int nc_get_keypress(void)
{
	int i;

	i=getch();

	// parse CURSES codes into our (IBM) scancodes
	switch(i)
	{
		// CR + LF == SAME thing... ack.. ohwell. curses
		// fucks CTRL-M (0x0D) to (0x0A) when it should return 0x0D
		case '\n':
		case '\r':
			i=0x0D;
			break;

		// my extended numpad 101/102 keyboard, pdcurses
		// gives 0x1D0 back for minus, 0x1D1 for plus.
		// convert them to plus + minus
		case 0x01D0:
			i='-';
			break;
		case 0x1D1:
			i='+';
			break;
	}
	return i;
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

static int nc_screen_init(uGlobalData *gdata)
{
	int i;

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

	intMaxHeight = gdata->screen->get_screen_height();
	intMaxWidth = gdata->screen->get_screen_width();

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

	init_style(STYLE_TITLE, gdata->clr_title_fg, gdata->clr_title_bg);					// title bar
	init_style(STYLE_NORMAL, gdata->clr_foreground, gdata->clr_background);				// default
	init_style(STYLE_HIGHLIGHT, gdata->clr_hi_foreground, gdata->clr_hi_background);	// highlight line

	gdata->screen->set_style(gdata, STYLE_NORMAL);
	gdata->screen->cls();

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

static int nc_screen_deinit(uGlobalData *gdata)
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

static void nc_set_style(uGlobalData *gdata, int style)
{
	switch(style)
	{
		case STYLE_NORMAL:
			intStyle = STYLE_NORMAL;
			setcolour(STYLE_NORMAL, styles[style].s_on);
			break;

		case STYLE_TITLE:
			intStyle = STYLE_TITLE;
			setcolour(STYLE_TITLE, styles[style].s_on);
			break;

		case STYLE_HIGHLIGHT:
			intStyle = STYLE_TITLE;
			setcolour(STYLE_HIGHLIGHT, styles[style].s_on);
			break;
	}
}

uScreenDriver screen_ncurses =
{
	nc_screen_init,		// init screen
	nc_screen_deinit,	// uninit
	nc_cls,				// clear scren
	nc_get_screen_height,
	nc_get_screen_width,
	nc_get_keypress,
	nc_print_string,
	nc_set_style,
	setcursor,
	erase_eol,
	nc_draw_frame,
	nc_init_style
};
