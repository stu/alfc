/****h* ALFC/LuaViewerAPI
 * FUNCTION
 *   The C<>Lua interface functions used in the Lua scripts in the viewer window
 *****
 */

#include "headers.h"
/*
 * comments
 * keywords
 * literals
 * colors
 */

static void FreeBufferLines(uViewFile *v)
{
	int i;

	for(i=0; i < v->intLineCount; i++)
	{
		free(v->lines[i].off);
	}
}


static uWindow* BuildViewerWindow(uGlobalData *gd)
{
	uWindow *w;

	w = calloc(1, sizeof(uWindow));

	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = 0;
	w->offset_col = 0;
	w->width = gd->screen->get_screen_width();
	w->height = gd->screen->get_screen_height() - 2;

	return w;
}

static DList* LoadLines(uViewFile *v, GetLine LoadLine)
{
	FILE *fp;
	int mx = 128;
	int c;
	uLine *l;
	char nw[32];

	c = 0;
	l = calloc(mx, sizeof(uLine));

	if(LoadLine != NULL)
	{
		int rc = 0;

		while(rc == 0)
		{
			l[c].off = calloc(1, 2048);

			rc = LoadLine(v->gd, c, l[c].off, 2048);

			if(rc == 0)
			{
				l[c].length = strlen((char*)l[c].off) + 4;
				l[c].off = realloc(l[c].off, l[c].length);

				if(v->intMaxCol < l[c].length)
					v->intMaxCol = l[c].length;

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
			else
				free(l[c].off);
		}
	}
	else
	{
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
				len	= 0;

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
	}

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
	char *buff;
	int q;
	int j;
	int tabs = v->tabsize;
	int width;

	width = v->w->screen->get_screen_width() + 16;
	if(width < 256)
		width = 256;

	buff = malloc(width);

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
	memset(buff, ' ', width);
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
		while(j < v->lines[ln].length && (p-buff) < width)
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
		v->w->screen->set_style(STYLE_VIEW_EDIT_EOL);
	else
		v->w->screen->set_style(STYLE_NORMAL);

	v->w->screen->print_abs(buff);
	v->w->screen->set_style(STYLE_NORMAL);
	
	free(buff);
}

static int DisplayStatus(uViewFile *v)
{
	char *buff;
	char *p;
	int	width;

	width = v->w->screen->get_screen_width() + 16;
	if(width < 256)
		width = 256;

	buff = malloc(width);

	memset(buff, ' ', width);
	memmove(buff, " Line: ", 7);

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

	if(v->intLineCount == 0)
		sprintf(p, "%3i%%",  (1 + v->intTLine + v->intHLine) * 100 / 1);
	else
		sprintf(p, "%3i%%",  (1 + v->intTLine + v->intHLine) * 100 / v->intLineCount );
	p=strchr(buff, 0);
	*p++ = ' ';

	if(v->crlf_type == eLine_DOS)
		sprintf(p,"DOS CRLF");
	else if(v->crlf_type == eLine_MAC)
		sprintf(p, "OLD MAC");
	else if(v->crlf_type == eLine_Unix)
		sprintf(p, "UNIX");
	else
		sprintf(p, "MIXED CR/LF");

	p = strchr(buff, 0x0);
	sprintf(p, " (%s)", v->ftype);

	p = strchr(buff, 0x0);
	sprintf(p, ", tab:%2i ", v->tabsize);

	p = strchr(buff, 0x0);
	sprintf(p, " (%s) ", v->intViewMode == eView_Hex ? "Hex" : "Text");

	v->w->screen->set_cursor(v->w->offset_row + v->w->height, 1 + v->w->offset_col + v->nwidth + 4);
	buff[v->w->width - (v->nwidth+2)] = 0;

	v->w->screen->set_style(STYLE_TITLE);
	v->w->screen->print_abs(buff);
	v->w->screen->set_style(STYLE_NORMAL);
	
	free(buff);

	return 0;
}


static int DisplayCLI(uViewFile *v)
{
	v->w->screen->set_style(STYLE_TITLE);
	v->w->screen->set_cursor(v->w->screen->get_screen_height()-1, 1);
	v->w->screen->print(" > ");

	v->w->screen->set_style(STYLE_NORMAL);
	v->w->screen->erase_eol();
	v->w->screen->set_cursor(v->w->screen->get_screen_height()-1, 4);

	v->w->screen->print(v->command);

	v->w->screen->set_cursor(v->w->screen->get_screen_height()-1, 4 + strlen(v->command));

	return 0;
}

static int DisplayFile(uViewFile *v)
{
	int i;
	int ln;
	char buff[256];
	int ostatus;

	ln = v->intTLine;

	ostatus = v->w->screen->get_updates();
	v->w->screen->set_updates(0);

	v->w->screen->set_style(STYLE_TITLE);
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

	v->w->screen->set_updates(ostatus);

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
		v->intTLine = v->intLineCount - wh  ;
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

int ExecuteViewerString(uViewFile *v, char *sn)
{
	char *outbuff;
	int maxsize;
	int rc = 0;

	lua_State *l;
	int vv;

	maxsize = strlen(sn);
	outbuff = malloc(64 + maxsize);

	if(outbuff == NULL)
	{
		LogError("No memory for buffer\n");
		return -1;
	}

	memmove(outbuff, sn, maxsize + 1);

	l = lua_open();
	RegisterViewerFuncs(v, l);

	vv = luaL_loadbuffer(l, (char*)outbuff, maxsize, "BUFFER");
	if(vv != 0)
	{
		LogError("cant load buffer, error %s\nBuffer = %s\n", lua_tostring(l, -1), sn);
		rc = -1;
	}
	else
	{
		vv = lua_pcall(l, 0, 0, 0);
		if(vv != 0)
		{
			LogError("lua error : %s\n", lua_tostring(l, -1));
			rc = -1;
		}
	}

	lua_close(l);
	free(outbuff);

	return rc;
}

static int ExecuteViewerScript(uViewFile *v, char *sn)
{
	char *outbuff;
	int maxsize;
	FILE *fp;
	char *fnx;

	int rc = 0;

	lua_State *l;
	int vv;

	fnx = ConvertDirectoryName(sn);

	fp = fopen(fnx, "r");
	if(fp == NULL)
	{
		LogError("cant load file : %s\n", fnx);
		free(fnx);
		return -1;
	}

	fseek(fp, 0x0L, SEEK_END);
	maxsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	outbuff = malloc(64 + maxsize);
	if(outbuff == NULL)
	{
		LogError("No memory for script %s\n", sn);
		fclose(fp);
		free(fnx);
		return -1;
	}

	fread(outbuff, 1, maxsize, fp);
	fclose(fp);

	l = lua_open();
	RegisterViewerFuncs(v, l);

	vv = luaL_loadbuffer(l, (char*)outbuff, maxsize, sn);
	if(vv != 0)
	{
		LogError("cant load file : %s, error %s\n", sn, lua_tostring(l, -1));
		rc = -1;
	}
	else
	{
		vv = lua_pcall(l, 0, 0, 0);
		if(vv != 0)
		{
			LogError("script : %s\n", sn);
			LogError("lua error : %s\n", lua_tostring(l, -1));
			rc = -1;
		}
	}

	lua_close(l);
	free(outbuff);

	return rc;
}


static void exec_internal_viewer_command(uViewFile *v, char *command)
{
	if(command == NULL)
		return;

	if(v->_GL == NULL)
		return;

	CallGlobalFunc(v->_GL, "CLIParse", "s", command);
}


static int LoadGlobalViewerScript(uViewFile *v, char *sn)
{
	uint32_t maxsize;
	FILE *fp;
	int vv;
	char *cpath;

	if(sn == NULL)
	{
		LogError("cant find global viewer script file. Will attempt to use globals.\n");
		return -1;
	}

	cpath = ConvertDirectoryName(sn);

	fp = fopen(cpath, "rb");
	if(fp == NULL)
	{
		free(cpath);
		LogError("cant load file : %s\n", cpath);
		return -1;
	}

	fseek(fp, 0x0L, SEEK_END);
	maxsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	v->gcode = calloc(1, 64 + maxsize);
	if(v->gcode == NULL)
	{
		free(cpath);
		LogError("No memory for script %s\n", cpath);
		fclose(fp);
		return -1;
	}
	fread(v->gcode, 1, maxsize, fp);
	fclose(fp);
	free(cpath);

	v->_GL = lua_open();
	RegisterViewerFuncs(v, v->_GL);

	vv = luaL_loadbuffer(v->_GL, v->gcode, maxsize, "GlobalLuaFuncs");
	if(vv != 0)
	{
		LogError("_gl lua error : %s\n", lua_tostring(v->_GL, -1));
		return -1;
	}

	// call to register globals
	vv = lua_pcall(v->_GL, 0, 0, 0);			/* call 'SetGlobals' with 0 arguments and 0 result */
	if(vv != 0)
	{
		LogError("_gl lua error : %s\n", lua_tostring(v->_GL, -1));
		return -1;
	}

	return 0;
}

static void ReleaseViewerMenu(uViewFile *v)
{
	FreeMenuData(v->gd->viewer_menu);
}

void ViewerDrawAll(uViewFile *v)
{
	v->w->screen->set_updates(0);

	DisplayFile(v);
	DrawMenuLine(v->w->screen, v->lstHotKeys);
	DisplayCLI(v);

	v->w->screen->set_updates(1);
}

// LoadLine is a callback for reading from a Lua table data structure.
// if its NULL we assume FN is a valid file, otherwise its the window title.
int ViewFile(uGlobalData *gd, char *fn, GetLine LoadLine)
{
	uViewFile *v;
	struct stat stats;
	int rc;
	int old_mode;
	uDirEntry *de;

	if(fn == NULL)
		return 1;

	stats.st_mode = 0;
	if(LoadLine == NULL)
	{
		ALFC_stat(fn, &stats);

		de = calloc(1, sizeof(uDirEntry));
		de->path = strdup("");
		de->name = strdup(fn);
		de->attrs = stats.st_mode;
		stats.st_mode = (uint16_t)ALFC_GetFileAttrs(de)&0xFFFF;
		free(de->path);
		free(de->name);
		free(de);
	}

	// don't do view on a directory.
	if(ALFC_IsDir(stats.st_mode) == 0)
		return 1;

	gd->screen->init_view_styles(gd->screen);

	if(LoadLine != NULL)
		AddHistory(gd, "View buffer : %s\n", fn);
	else
		AddHistory(gd, "View file : %s\n", fn);

	old_mode = gd->mode;
	gd->mode = eMode_Viewer;

	v = calloc(1, sizeof(uViewFile));
	v->gd = gd;
	v->w = BuildViewerWindow(gd);
	v->fn = ConvertDirectoryName(fn);
	v->intViewMode = eView_Text;
	v->intTLine = 0;
	v->intHLine = 0;
	v->tabsize = 8;
	v->ftype = strdup("");

	v->lstHotKeys = malloc(sizeof(DList));
	dlist_init(v->lstHotKeys, FreeKey);

	rc = LoadGlobalViewerScript(v, INI_get(v->gd->optfile, "scripts", "viewer_funcs"));
	if(rc != 0)
		rc = LoadGlobalViewerScript(v, "$HOME/.alfc/global.lua");

	gd->screen->set_style(STYLE_NORMAL);
	//v->w->screen->cls();
	v->w->screen->window_clear(v->w);

	gd->screen->set_style(STYLE_TITLE);
	v->w->screen->draw_border(v->w);

	// incase its a HUUUUUUUUUUGE file, throw up a vague title screen
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(1, ((gd->screen->get_screen_width() - (strlen(gstr_WindowTitle) - 8))/2));
	gd->screen->print(gstr_WindowTitle);

	if(ALFC_IsDir(stats.st_mode) != 0)
	{
		LoadLines(v, LoadLine);
		ViewerDrawAll(v);

		v->quit_flag = 0;
		while(v->quit_flag == 0 && GetQuitAppFlag() == 0)
		{
			uint32_t key;

			key = gd->screen->get_keypress();
			uKeyBinding *kb;
			int mnu;

			v->w->screen->set_cursor(v->w->screen->get_screen_height()-1, 4 + strlen(v->command));

			mnu = ScanMenuOpen(v->gd, key);

			kb = ScanKey(v->lstHotKeys, key);
			if(kb != NULL)
			{
				AddHistory(v->gd, kb->sCommand);

				// exec string via lua...
				if(kb->sCommand[0] == ':')
					exec_internal_viewer_command(v, kb->sCommand);
				else if(kb->sCommand[0] == '@')
				{
					char *fn = ConvertDirectoryName(kb->sCommand+1);
					ExecuteViewerScript(v, fn);
					free(fn);
				}
				else
				{
					ExecuteGlobalViewerString(v, kb->sCommand);
				}
			}
			else
			{
				switch(key)
				{
					
					case ALFC_KEY_ESCAPE:
						v->quit_flag = 1;
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

					case ALFC_KEY_ENTER:
						if(v->command_length > 0)
						{
							AddHistory(gd, v->command);

							// exec string via lua...
							if(v->command[0] == ':')
								exec_internal_viewer_command(v, v->command);
							else if(v->command[0] == '@')
							{
								char *fn = ConvertDirectoryName(v->command+1);
								ExecuteViewerScript(v, fn);
								free(fn);
							}
							else
							{
								ExecuteViewerString(v, v->command);
							}

							v->command_length = 0;
							v->command[v->command_length] = 0;
						}
						break;

					default:
						if(key == 0)
							break;

						// terminal could send ^H (0x08) or ASCII DEL (0x7F)
						if ((key == ALFC_KEY_BACKSPACE || key == ALFC_KEY_DEL) || (key >= ' ' && key <= 0x7F))
						{
							if ((key == ALFC_KEY_DEL || key == ALFC_KEY_BACKSPACE))
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
				DisplayCLI(v);
			}


			if(v->redraw == 1)
			{
				DisplayFile(v);
				DisplayCLI(v);
				v->redraw = 0;
			}
		}
	}

	ReleaseViewerMenu(v);

	if(v->lstHotKeys != NULL)
	{
		dlist_destroy(v->lstHotKeys);
		free(v->lstHotKeys);
	}

	if(v->gcode != NULL)
	{
		free(v->gcode);
	}

	if(v->_GL != NULL)
		lua_close(v->_GL);

	if(v->lines != NULL && LoadLine == NULL)
		free(v->lines);
	else
	{
		FreeBufferLines(v);
		free(v->lines);
	}

	if(v->data != NULL)
		free(v->data);

	if(v->fn != NULL)
		free(v->fn);

	if(v->ftype != NULL)
		free(v->ftype);

	free(v->w);
	free(v);

	gd->mode = old_mode;
	return 0;
}


#define VIEWERDATA "uViewerData"
static const char *uViewerData_Key = VIEWERDATA;
uViewFile* GetViewerData(lua_State *L)
{
	uViewFile *v;

	lua_pushlightuserdata(L, (void *)&uViewerData_Key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	v = (uViewFile*)lua_touserdata(L, -1);

	return v;
}

int RegisterViewerData(uViewFile *v, lua_State *l)
{
	lua_pushlightuserdata(l, (void*)&uViewerData_Key);	/* push address */
	lua_pushlightuserdata(l, v);
	lua_settable(l, LUA_REGISTRYINDEX);

	return 1;
}

int gmev_SetTabSize(lua_State *L)
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


/****f* LuaViewerAPI/QuitViewer
* FUNCTION
*	Sets the flag to tell the application to quit.
* SYNOPSIS
SetQuitAppFlag()
* INPUTS
*	o None
* RESULTS
*   o None
* NOTES
* 	This does not cause the app to quit there and then, it will most likely fiinish whatever
* 	processing it was doing and once it gets back to the main screen, will quit out.
* AUTHOR
*	Stu George
******
*/
int gmev_QuitViewer(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);
	v->quit_flag = 1;

	return 0;
}

/****f* LuaViewerAPI/BindKey
* FUNCTION
*	Binds a key to a command string which is passed intnernally just as if you had typed
* 	it on the cli bar (eg: ":q")
* SYNOPSIS
error = BindKey(key, title, command)
* INPUTS
*	o key (constant) -- (need to insert key listing)
*	o title (string) -- what shows up on the command bar
*	o command (string) -- command string the key invokes
* RESULTS
*	error (integer):
*	o 0 -- OK
*	o -1 -- Key already bound
* SEE ALSO
*	SetFilter, AddFilter, SetGlob
* AUTHOR
*	Stu George
******
*/
int gmev_BindKey(lua_State *L)
{
	struct lstr kstring;
	struct lstr ktitle;
	uint32_t key;
	uViewFile *v;
	uKeyBinding *kb;

	v = GetViewerData(L);
	assert(v != NULL);

	key = luaL_checknumber(L, 1);
	GET_LUA_STRING(ktitle, 2);
	GET_LUA_STRING(kstring, 3);

	kb = ScanKey(v->lstHotKeys, key);
	if(kb != NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	kb = malloc(sizeof(uKeyBinding));

	kb->key = key;
	kb->sCommand = strdup(kstring.data);
	kb->sTitle = strdup(ktitle.data);

	dlist_ins(v->lstHotKeys, kb);

	lua_pushnumber(L, 0);
	return 1;
}

int gmev_About(lua_State *L)
{
	uGlobalData *gd;
	uViewFile *v;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = GetViewerData(L);
	assert(v != NULL);

	about_window(gd);

	v->redraw = 1;

	return 0;
}

int gmev_GetCurrentLineNumber(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	lua_pushnumber(L, 1 + v->intTLine + v->intHLine);
	return 1;
}

int gmev_GoToLine(lua_State *L)
{
	uViewFile *v;

	v = GetViewerData(L);
	assert(v != NULL);

	int wh = v->w->height - 3;
	int q;
	int l;

	l = luaL_checknumber(L, 1);
	l -= 1;

	if(l < 1 || l >= v->intLineCount)
		return 0;

	if(l == v->intHLine)
		return 0;

	if(l > v->intTLine && l < (v->intTLine+wh)-4 )
	{
		v->intHLine = l;
		v->redraw = 1;
		return 0;
	}

	// build new topline
	q = l - (wh / 2);

	if(q < 0)
	{
		v->intTLine = 0;
		v->intHLine = l;
		v->redraw = 1;
		return 0;
	}

	v->intTLine = q;
	v->intHLine =  (wh / 2);
	v->redraw = 1;

	return 0;
}

int gmev_ViewedFilename(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	lua_pushstring(L, v->fn);

	return 1;
}

int gmev_SetFileType(lua_State *L)
{
	struct lstr ftype;
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	GET_LUA_STRING(ftype, 1);

	if(v->ftype != NULL)
		free(v->ftype);

	v->ftype = strdup(ftype.data);

	return 0;
}

int gmev_GetFileType(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	assert(v != NULL);

	lua_pushstring(L, v->ftype);
	return 1;
}

int ViewerDrawAllLua(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	ViewerDrawAll(v);

	return 0;
}

int gmev_ViewerDrawAll(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	ViewerDrawAll(v);

	return 0;
}

/****f* LuaViewerAPI/GetViewMode
* FUNCTION
*	Returns the current active viewing mode.
* SYNOPSIS
mode = GetViewMode()
* RESULTS
*	mode (integer):
*	o eView_Text -- Sets text mode
*	o eView_Hex -- Sets hex mode
* SEE ALSO
*	SetViewMode, GetViewMode
* AUTHOR
*	Stu George
******
*/
int gmev_GetViewMode(lua_State *L)
{
	uViewFile *v;
	v = GetViewerData(L);
	lua_pushnumber(L, v->intViewMode);

	return 1;
}

/****f* LuaViewerAPI/SetViewMode
* FUNCTION
*	Sets the current view mode to either text or hex.
* SYNOPSIS
SetViewMode(eView_Hex)
* INPUTS
*	mode (integer):
*	o eView_Text -- Sets text mode
*	o eView_Hex -- Sets hex mode
* RESULTS
*   o Display will switch to specified mode.
* SEE ALSO
*	SetViewMode, GetViewMode
* AUTHOR
*	Stu George
******
*/
int gmev_SetViewMode(lua_State *L)
{
	uViewFile *v;
	int mode;

	v = GetViewerData(L);

	mode = luaL_checknumber(L, 1);
	if(!(mode == eView_Text || mode == eView_Hex))
		return luaL_error(L, "Unknown view mode");

	v->intViewMode = mode;
	v->redraw = 1;

	return 0;
}

