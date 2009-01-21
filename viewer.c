#include "headers.h"

enum eViewMode
{
	eView_Text = 1,
	eView_Hex
};

typedef struct udtLine
{
	uint32_t	length;
	uint8_t		*off;
} uLine;

typedef struct udtViewFile
{
	char	*fn;
	uWindow *w;

	uint8_t	*data;
	int		intLineCount;
	uLine	*lines;

	int		intViewMode;
	int		intTLine;
	int		intHLine;

	int		nwidth;
} uViewFile;


static uWindow* BuildViewerWindow(uGlobalData *gd)
{
	uWindow *w;

	w = calloc(1, sizeof(uWindow));

	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = 0;
	w->offset_col = 0;
	w->width = gd->screen->get_screen_width();
	w->height = gd->screen->get_screen_height();

	return w;
}

static DList* LoadLines(uViewFile *v)
{
	FILE *fp;
	int mx = 128;
	int c;
	uLine *l;
	char nw[32];

	c = 0;
	l = calloc(mx, sizeof(uLine));

	fp = fopen(v->fn, "rb");
	if(fp != NULL)
	{
		// PORTABILITY: 32bit file size??
		uint32_t len;
		uint32_t off;

		fseek(fp, 0x0L, SEEK_END);
		len = ftell(fp);
		fseek(fp, 0x0L, SEEK_SET);

		v->data = calloc(1, 64 + len);
		if(v->data != NULL)
		{
			fread(v->data, 1, len, fp);
		}
		else
			len = 0;

		fclose(fp);

		off = 0;
		while(off < len)
		{
			int count;

			// create line
			l[c].off = v->data + off;
			count = 0;

			while(off < len)
			{
				if( v->data[off] == 0x0D && v->data[off + 1] == 0x0A)
				{
					off += 2;
					count += 2;
					break;
				}
				else if( v->data[off] == 0x0D || v->data[off] == 0x0A)
				{
					off += 1;
					count += 1;
					break;
				}
				else
				{
					off += 1;
					count += 1;
				}
			}

			l[c].length = count;

			c += 1;

			if(c == mx)
			{
				mx = mx * 2;
				l = realloc(l, mx * sizeof(uLine));
				if(l == NULL)
				{
					c = 0;
					break;
				}
			}
		}
	}
	else
		LogInfo("Failed to open %s\n", v->fn);

	v->intLineCount = c;
	v->lines = l;


	sprintf(nw, "%i", v->intLineCount < 999 ? 999 : 4 + v->intLineCount);
	v->nwidth = strlen(nw);
	if(v->nwidth < 3) v->nwidth = 3;

	return 0;
}

static void PrintLine(uViewFile *v, int ln, int i)
{
	char *p;
	char buff[4096];
	int q;
	int j;
	int tabs = 4;

	// Feeling lazy
	switch(v->nwidth)
	{
		case 1:
		case 2:
		case 3:
			sprintf(buff, "%3i", 1+ ln);
			break;
		case 4:
			sprintf(buff, "%4i", 1+ln);
			break;
		case 5:
			sprintf(buff, "%5i", 1+ln);
			break;
		case 6:
			sprintf(buff, "%6i", 1+ln);
			break;
		case 7:
			sprintf(buff, "%7i", 1+ln);
			break;
		case 8:
			sprintf(buff, "%8i", 1+ln);
			break;
		case 9:
			sprintf(buff, "%9i", 1+ln);
			break;
		case 10:
			sprintf(buff, "%10i", 1+ln);
			break;
		default:
			sprintf(buff, "%11i", 1+ln);
			break;
	}
	v->w->screen->set_style(STYLE_TITLE);
	v->w->screen->set_cursor(2 + i + v->w->offset_row, 1 + v->w->offset_col );
	v->w->screen->print_abs(buff);
	v->w->screen->set_cursor(2 + i + v->w->offset_row, 1 + v->w->offset_col + v->nwidth);
	v->w->screen->print_vline();

	p = buff;
	//*p = 0;
	memset(buff, ' ', 4096);
	if(ln < v->intLineCount)
	{
		p = buff;
		j=0;

		while(j < v->lines[ln].length && j < 4000)
		{
			char c;
			c = v->lines[ln].off[j];
			switch(c)
			{
				case 0x0A:
				case 0x0D:
					break;

				case 0x09:
					for(q=0; q<tabs; q++)
						*p++ = ' ';
					break;

				default:
					*p++ = c;
					break;
			}

			j += 1;
		}
	}
	else if(ln == v->intLineCount)
	{
		sprintf(buff, "*** END OF FILE ***");
	}
	else if(ln == v->intLineCount + 1)
	{
		// this clears the end of file message
		//sprintf(buff, "                    ");
	}

	v->w->screen->set_cursor(2 + i + v->w->offset_row, 1 + v->w->offset_col + v->nwidth + 1);
	buff[v->w->width - (v->nwidth+2)] = 0;

	if(v->intHLine == i)
		v->w->screen->set_style(STYLE_HIGHLIGHT);
	else if(ln == v->intLineCount)
		v->w->screen->set_style(STYLE_EDIT_EOL);
	else
		v->w->screen->set_style(STYLE_NORMAL);

	v->w->screen->print_abs(buff);
	v->w->screen->set_style(STYLE_NORMAL);
}

static int DisplayFile(uViewFile *v)
{
	int i;
	int ln;
	char buff[256];

	ln = v->intTLine;

	v->w->offset_col += v->nwidth;
	v->w->width -= v->nwidth;
	v->w->screen->draw_border(v->w);
	v->w->offset_col -= v->nwidth;
	v->w->width += v->nwidth;

	v->w->screen->set_style(STYLE_TITLE);
	sprintf(buff, "[ %s ]", v->fn);
	v->w->screen->set_cursor(1 + v->w->offset_row, v->nwidth + 3 + v->w->offset_col );
	v->w->screen->print_abs(buff);

	sprintf(buff,"                ");
	buff[v->nwidth] = 0;
	v->w->screen->set_cursor(1 + v->w->offset_row, 1);
	v->w->screen->print_abs(buff);

	v->w->screen->set_cursor(0 + v->w->offset_row + v->w->height, 1);
	v->w->screen->print_abs(buff);

	for(i=0; i<v->w->height-2; i++)
	{
		PrintLine(v, ln, i);
		ln += 1;
	}

	return 0;
}

static int vw_scroll_down(uViewFile *v)
{
	int wh = v->w->height - 3;

	if( v->intHLine < wh)
	{
		if(v->intHLine < wh - 4)
		{
			if(v->intHLine < v->intLineCount - 1)
			{
				v->intHLine += 1;
				PrintLine(v, v->intTLine + v->intHLine - 1, v->intHLine - 1);
				PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
			}
			else
			{
				// scroll no more..
			}
		}
		else
		{
			// scroll page by tl!
			if(v->intHLine + v->intTLine + 1 < v->intLineCount)
			{
				v->intTLine += 1;
				DisplayFile(v);
			}
		}
	}

	return 0;
}

static int vw_scroll_up(uViewFile *v)
{
	if(v->intHLine > 4)
	{
		v->intHLine -= 1;
		PrintLine(v, v->intTLine + v->intHLine + 1, v->intHLine + 1);
		PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
	}
	else
	{
		if(v->intTLine == 0 && v->intHLine > 0)
		{
			v->intHLine -= 1;
			PrintLine(v, v->intTLine + v->intHLine + 1, v->intHLine + 1);
			PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
		}
		else
		{
			if(v->intTLine > 0)
			{
				v->intTLine -= 1;
				DisplayFile(v);
			}
		}
	}

	return 0;
}

static int vw_scroll_page_down(uViewFile *v)
{
	int wh = v->w->height - 3;

	if( v->intHLine < wh - 4)
	{
		if( wh - 4 < v->intLineCount)
		{
			int j;

			j = v->intHLine;
			v->intHLine = wh - 4;
			PrintLine(v, v->intTLine + j, j);
			PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
		}
	}
	else
	{
		if(v->intTLine + v->intHLine + wh-4 < v->intLineCount)
		{
			v->intTLine += wh - 4;
			DisplayFile(v);
		}
		else
		{
			wh -= 1;
			v->intTLine = v->intLineCount - wh ;
			v->intHLine = wh - 1;
			DisplayFile(v);
		}
	}

	return 0;
}

static int vw_scroll_page_up(uViewFile *v)
{
	int wh = v->w->height - 3;
	int j;

	if(v->intHLine > 4)
	{
		j = v->intHLine;
		v->intHLine = 4;
		PrintLine(v, v->intTLine + j, j);
		PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
	}
	else
	{
		if(v->intTLine > 0)
		{
			if(v->intTLine - wh - 4 >= 0)
			{
				v->intTLine -= wh - 4;
				DisplayFile(v);
			}
			else
			{
				v->intTLine = 0;
				v->intHLine = 0;
				DisplayFile(v);
			}
		}
		else
		{
			if(v->intHLine > 0)
			{
				j = v->intHLine;
				v->intHLine = 0;
				PrintLine(v, v->intTLine + j, j);
				PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
			}
		}
	}

	return 0;
}


int ViewFile(uGlobalData *gd, char *fn)
{
	uViewFile *v;
	int intFlag;

	v = calloc(1, sizeof(uViewFile));
	v->w = BuildViewerWindow(gd);
	v->fn = ConvertDirectoryName(fn);
	v->intViewMode = eView_Text;
	v->intTLine = 0;
	v->intHLine = 0;

	gd->screen->set_style(STYLE_NORMAL);
	v->w->screen->cls();
	v->w->screen->draw_border(v->w);

	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(1, ((gd->screen->get_screen_width() - (strlen(" Welcome to Another Linux File Commander ") - 8))/2));
	gd->screen->print(" Welcome to Another Linux File Commander ");

	LoadLines(v);

	DisplayFile(v);

	intFlag = 0;
	while(intFlag == 0)
	{
		uint32_t key;

		key = gd->screen->get_keypress();

		switch(key)
		{
			case ALFC_KEY_F00 + 10:
				intFlag = 1;
				break;

			case ALFC_KEY_DOWN:
				vw_scroll_down(v);
				break;

			case ALFC_KEY_UP:
				vw_scroll_up(v);
				break;

			case ALFC_KEY_PAGE_DOWN:
				vw_scroll_page_down(v);
				break;

			case ALFC_KEY_PAGE_UP:
				vw_scroll_page_up(v);
				break;
		}
	}


	if(v->lines != NULL)
		free(v->lines);

	if(v->data != NULL)
		free(v->data);

	if(v->fn != NULL)
		free(v->fn);

	free(v->w);
	free(v);
	return 0;
}


#define VIEWERDATA "uViewerData"
static const char *uViewerData_Key = VIEWERDATA;
static uViewFile* GetViewerData(lua_State *L)
{
	uViewFile *v;

	lua_pushlightuserdata(L, (void *)&uViewerData_Key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	v = (uViewFile*)lua_touserdata(L, -1);

	return v;
}

int RegisterViewerData(uViewFile *v, lua_State *l)
{
	lua_pushlightuserdata(l, (void*)&uViewerData_Key);  /* push address */
	lua_pushlightuserdata(l, v);
	lua_settable(l, LUA_REGISTRYINDEX);

	return 1;
}

int gme_SwitchToHexView(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	v->intViewMode = eView_Hex;

	return 0;
}

