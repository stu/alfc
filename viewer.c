#include "headers.h"
/*

* comments
* keywords
* literals
* colors


 */

enum eLineFeed
{
	eLine_Unix = 1,		// LF - 0x0A
	eLine_DOS = 2,			// CRLF - 0x0D, 0x0A
	eLine_MAC = 4,			// CR - 0x0D
	eLine_Mixed = 8
};

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
	int		intColOffset;
	int		intMaxCol;
	int		tabsize;
	int		nwidth;
	int		redraw;

	int		crlf_type;

	int		command_length;
	char	command[128];
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
	w->height = gd->screen->get_screen_height() - 1;

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
					if(v->crlf_type == eLine_DOS || v->crlf_type == 0)
						v->crlf_type |= eLine_DOS;
					else
						v->crlf_type |= eLine_Mixed;

					off += 2;
					count += 2;
					break;
				}
				else if( v->data[off] == 0x0D || v->data[off] == 0x0A)
				{
					if(v->data[off] == 0x0D)
					{
						if(v->crlf_type == eLine_MAC || v->crlf_type == 0)
							v->crlf_type |= eLine_MAC;
						else
							v->crlf_type |= eLine_Mixed;
					}
					else
					{
						if(v->crlf_type == eLine_Unix || v->crlf_type == 0)
							v->crlf_type |= eLine_Unix;
						else
							v->crlf_type |= eLine_Mixed;
					}

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
			if(v->intMaxCol < count)
				v->intMaxCol = count;

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
	int tabs = v->tabsize;

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
		int xoff;

		p = buff;
		j=0;
		xoff = 0;

		while(xoff < v->intColOffset && j < v->lines[ln].length)
		{
			switch(v->lines[ln].off[j])
			{
				case 0x0A:
				case 0x0D:
					break;

				case 0x09:
					{
						int t;

						t = xoff % tabs;
						for(q = t; q<tabs && xoff < v->intColOffset; q++)
							xoff += 1;
					}
					break;

				default:
					if(v->lines[ln].off[j] >= ' ')
						xoff += 1;
					break;
			}

			j += 1;
		}

		//j = v->intColOffset;
		while(j < v->lines[ln].length && (p-buff) < 4000)
		{
			uint8_t c;
			c = v->lines[ln].off[j];
			switch(c)
			{
				case 0x0A:
				case 0x0D:
					break;

				case 0x09:
					{
						int t;

						t = xoff % tabs;
						for(q = t; q<tabs; q++)
						{
							*p++ = ' ';
							xoff += 1;
						}
					}
					break;

				default:
					if(c >= ' ')
					{
						*p++ = c;
						xoff += 1;
					}
					break;
			}

			j += 1;
		}
	}
	else if(ln == v->intLineCount)
	{
		memmove(buff, "*** END OF FILE ***", 19);
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

static int DisplayStatus(uViewFile *v)
{
	char buff[4096];
	char *p;

	memset(buff, ' ', 4096);

	memmove(buff, " Line: ", 7);

	//sprintf(buff + 40, "%i/%i, %i%%", 1 + v->intTLine + v->intHLine, v->intLineCount, (1 + v->intTLine + v->intHLine) * 100 / v->intLineCount );
	sprintf(buff + 40, "%i/%i, ", 1 + v->intTLine + v->intHLine, v->intLineCount);

	//(v->nwidth * 2) + 4
	// +3 buffering
	memmove(buff + 7 + (v->nwidth * 2) + 3 - strlen(buff+40) , buff+40, strlen(buff+40)+1);
	memset(buff+40, ' ', 40);

	p=strchr(buff, 0);
	*p = ' ';

	sprintf(p, "+%-3i", v->intColOffset);

	p=strchr(buff, 0);
	*p++ = ' ';

	sprintf(p, "%3i%%",  (1 + v->intTLine + v->intHLine) * 100 / v->intLineCount );
	p=strchr(buff, 0);
	*p++ = ' ';

	if(v->crlf_type == eLine_DOS)
		sprintf(p," DOS CRLF ");
	else if(v->crlf_type == eLine_MAC)
		sprintf(p, " OLD MAC ");
	else if(v->crlf_type == eLine_Unix)
		sprintf(p, " UNIX ");
	else
		sprintf(p, " MIXED CR/LF ");

	p = strchr(buff, 0x0);
	//p++ = ' ';
	sprintf(p, ", tab: %-2i", v->tabsize);


	v->w->screen->set_cursor(v->w->offset_row + v->w->height, 1 + v->w->offset_col + v->nwidth + 4);
	buff[v->w->width - (v->nwidth+2)] = 0;

	v->w->screen->set_style(STYLE_TITLE);
	v->w->screen->print_abs(buff);
	v->w->screen->set_style(STYLE_NORMAL);

	return 0;
}

static int DisplayCLI(uViewFile *v)
{
	v->w->screen->set_style(STYLE_TITLE);
	v->w->screen->set_cursor(v->w->screen->get_screen_height(), 1);
	v->w->screen->print(" > ");

	v->w->screen->set_style(STYLE_NORMAL);
	v->w->screen->erase_eol();
	v->w->screen->set_cursor(v->w->screen->get_screen_height(), 4);

	v->w->screen->print(v->command);

	return 0;
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

	DisplayStatus(v);

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
				DisplayStatus(v);
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
		DisplayStatus(v);
	}
	else
	{
		if(v->intTLine == 0 && v->intHLine > 0)
		{
			v->intHLine -= 1;
			PrintLine(v, v->intTLine + v->intHLine + 1, v->intHLine + 1);
			PrintLine(v, v->intTLine + v->intHLine, v->intHLine);
			DisplayStatus(v);
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
			DisplayStatus(v);
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
			wh -= 3;
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
		DisplayStatus(v);
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
				DisplayStatus(v);
			}
		}
	}

	return 0;
}

static int vw_scroll_home(uViewFile *v)
{
	if(v->intTLine > 0 || v->intHLine > 0)
	{
		v->intTLine = 0;
		v->intHLine = 0;
		DisplayFile(v);
	}

	return 0;
}

static int vw_scroll_end(uViewFile *v)
{
	int wh = v->w->height - 3;

	if(v->intLineCount < wh)
	{
		if(v->intHLine < v->intLineCount - 1)
		{
			v->intHLine = v->intLineCount - 1;
			DisplayFile(v);
		}
	}
	else if(!(v->intTLine == v->intLineCount - wh - 1 && v->intHLine == wh-2))
	{
		wh -= 3;
		v->intTLine = v->intLineCount - wh	;
		v->intHLine = wh - 1;
		DisplayFile(v);
	}

	return 0;
}

static int vw_scroll_right(uViewFile *v)
{
	if(v->intColOffset < v->intMaxCol - (v->tabsize *2))//+ v->w->width/2)
	{
		v->intColOffset += v->tabsize;
		DisplayFile(v);
	}

	return 0;
}

static int vw_scroll_left(uViewFile *v)
{
	if(v->intColOffset > 0)
	{
		v->intColOffset -= v->tabsize;
		if(v->intColOffset < 0)
			v->intColOffset = 0;
		DisplayFile(v);
	}

	return 0;
}

int ViewFile(uGlobalData *gd, char *fn)
{
	uViewFile *v;
	int intFlag;
	struct stat stats;

	v = calloc(1, sizeof(uViewFile));
	v->w = BuildViewerWindow(gd);
	v->fn = ConvertDirectoryName(fn);
	v->intViewMode = eView_Text;
	v->intTLine = 0;
	v->intHLine = 0;
	v->tabsize = 8;

	gd->screen->set_style(STYLE_NORMAL);
	v->w->screen->cls();
	v->w->screen->draw_border(v->w);

	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(1, ((gd->screen->get_screen_width() - (strlen(" Welcome to Another Linux File Commander ") - 8))/2));
	gd->screen->print(" Welcome to Another Linux File Commander ");

	stat(fn, &stats);
	if( S_ISDIR(stats.st_mode) == 0)
	{
		LoadLines(v);
		DisplayFile(v);

		intFlag = 0;
		while(intFlag == 0)
		{
			uint32_t key;

			DisplayCLI(v);
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

				case ALFC_KEY_END:
					vw_scroll_end(v);
					break;

				case ALFC_KEY_HOME:
					vw_scroll_home(v);
					break;

				case ALFC_KEY_LEFT:
					vw_scroll_left(v);
					break;

				case ALFC_KEY_RIGHT:
					vw_scroll_right(v);
					break;

				default:
					// terminal could send ^H (0x08) or ASCII DEL (0x7F)
					if(key == ALFC_KEY_DEL || (key >= ' ' && key <= 0x7F))
					{
						if(key == ALFC_KEY_DEL)
						{
							if(v->command_length > 0)
							{
								v->command_length -= 1;
								v->command[v->command_length] = 0;
							}
						}
						else
						{
							if(v->command_length < v->w->screen->get_screen_width() - 10)
							{
								v->command[v->command_length++] = key;
								v->command[v->command_length] = 0;
							}
						}

						DisplayCLI(v);
					}
					else
						LogInfo("Unknown key 0x%04x\n", key);
					break;
			}
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
	v->redraw = 1;

	return 0;
}

int gme_SetTabSize(lua_State *L)
{
	int q;
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	q = luaL_checknumber(L, 1);

	if(q < 2) q = 2;
	if(q > 16) q = 16;

	v->tabsize = q;
	v->redraw = 1;

	return 0;
}
