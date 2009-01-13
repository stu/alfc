#include "headers.h"

static int intCurCol;
static int intCurRow;
static int intMaxHeight;
static int intMaxWidth;

static int intMaxColourPairs;


static int nc_convert_colour(int color)
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


static void dr_outchar(uint8_t s, uint8_t last_colour)
{
	if(last_colour != 0)
	{
		if(last_colour>=CLR_DK_GREY)
			setcolour(last_colour-8, A_BOLD);
		else
			setcolour(last_colour, A_NORMAL);
	}

	mvaddch(intCurRow, intCurCol, s);
}

static void nc_print_string(const char *s)
{
	while(*s!=0x0)
	{
		switch(*s)
		{
			case '}':
				setcolour(STYLE_NORMAL, A_NORMAL);
				break;
			case '{':
				setcolour(STYLE_NORMAL, A_BOLD);
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
					dr_outchar(' ', 0);
				}
				break;

			case 0x09:							// tab
				intCurCol = ((intCurCol+8)>>3)<<3;

				if(intCurCol >= intMaxWidth)
					intCurCol=intMaxWidth-1;
				break;

			default:
				dr_outchar(*s, 0);
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

static int nc_screen_init(uGlobalData *gdata)
{
	int i;

	// initialise ncurses
	initscr();
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
	for(i=1; i>0; i++)
		if(init_pair((char)i, CLR_WHITE, CLR_BLACK)==ERR)
			i=-1;

	intMaxColourPairs = i;

	LogInfo("Can handle %i colourpairs\n", intMaxColourPairs);

	init_pair(CLR_BLACK,	COLOR_BLACK,	COLOR_BLACK);
	init_pair(CLR_RED,		COLOR_RED,		COLOR_BLACK);
	init_pair(CLR_GREEN,	COLOR_GREEN,	COLOR_BLACK);
	init_pair(CLR_BROWN,	COLOR_YELLOW,	COLOR_BLACK);
	init_pair(CLR_BLUE,		COLOR_BLUE,		COLOR_BLACK);
	init_pair(CLR_MAGENTA,	COLOR_MAGENTA,	COLOR_BLACK);
	init_pair(CLR_CYAN,		COLOR_CYAN,		COLOR_BLACK);
	init_pair(CLR_GREY,		COLOR_WHITE,	COLOR_BLACK);


	init_pair(STYLE_TITLE,
				nc_convert_colour(gdata->clr_title_fg),
				nc_convert_colour(gdata->clr_title_bg));		// title bar

	init_pair(STYLE_NORMAL,
				nc_convert_colour(gdata->clr_foreground),
				nc_convert_colour(gdata->clr_background));		// default

	gdata->screen->set_style(gdata, STYLE_NORMAL);

	gdata->screen->cls();
	LogWrite("NCurses init done\n");


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
	reset_screen();
	curs_set(i);
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
			setcolour(STYLE_NORMAL, A_NORMAL);
			//setcolour(nc_convert_colour(gdata->clr_foreground), nc_convert_colour(gdata->clr_background));
			break;

		case STYLE_TITLE:
			setcolour(STYLE_TITLE, A_NORMAL);
			//setcolour(nc_convert_colour(gdata->clr_title_fg), nc_convert_colour(gdata->clr_title_bg));
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
};
