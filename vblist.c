/* Add some functionality and clone a bit of Vernon Buerg's famous LIST.COM */

#include "headers.h"

#define OFF_COL		12

void SetListFiles(uListFile *lf, DList *lstF)
{
	lf->lstFiles = lstF;
	lf->gd->lstLeft = lf->lstFiles;
}

void SetListFullFiles(uListFile *lf, DList *lstF)
{
	lf->lstFullFiles = lstF;
	lf->gd->lstFullLeft = lf->lstFullFiles;
}

static int get_scroll_depth(uWindow *w)
{
	if (dlist_size(GetActList(w->gd)) < w->height - 2)
		return dlist_size(GetActList(w->gd));
	else
		return(w->height - 2) -	4;
}

int ExecuteListString(uListFile *lf, char *sn)
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
	RegisterListFuncs(lf, l);

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

static int ExecuteListScript(uListFile *lf, char *sn)
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
	RegisterListFuncs(lf, l);

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


static void exec_internal_list_command(uListFile *lf, char *command)
{
	if(command == NULL)
		return;

	if(lf->_GL == NULL)
		return;

	CallGlobalFunc(lf->_GL, "CLIParse", "s", command);
}

static uWindow* BuildListWindow(uGlobalData *gd)
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

static int LoadGlobalListScript(uListFile *lf, char *sn)
{
	uint32_t maxsize;
	FILE *fp;
	int vv;
	char *cpath;

	if(sn == NULL)
	{
		LogError("cant find global list script file. Will attempt to use globals.\n");
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

	lf->gcode = calloc(1, 64 + maxsize);
	if(lf->gcode == NULL)
	{
		free(cpath);
		LogError("No memory for script %s\n", cpath);
		fclose(fp);
		return -1;
	}
	fread(lf->gcode, 1, maxsize, fp);
	fclose(fp);
	free(cpath);

	lf->_GL = lua_open();
	RegisterListFuncs(lf, lf->_GL);

	vv = luaL_loadbuffer(lf->_GL, lf->gcode, maxsize, "GlobalLuaFuncs");
	if(vv != 0)
	{
		LogError("_gl lua error : %s\n", lua_tostring(lf->_GL, -1));
		return -1;
	}

	// call to register globals
	vv = lua_pcall(lf->_GL, 0, 0, 0);			/* call 'SetGlobals' with 0 arguments and 0 result */
	if(vv != 0)
	{
		LogError("_gl lua error : %s\n", lua_tostring(lf->_GL, -1));
		return -1;
	}

	return 0;
}

static void ReleaseListMenu(uListFile *lf)
{
	FreeMenuData(lf->gd->list_menu);
}


#define LISTDATA "uListData"
static const char *uListData_Key = LISTDATA;
uListFile* GetListData(lua_State *L)
{
	uListFile *lf;

	lua_pushlightuserdata(L, (void *)&uListData_Key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lf = (uListFile*)lua_touserdata(L, -1);

	return lf;
}

int RegisterListData(uListFile *v, lua_State *l)
{
	lua_pushlightuserdata(l, (void*)&uListData_Key);	/* push address */
	lua_pushlightuserdata(l, v);
	lua_settable(l, LUA_REGISTRYINDEX);

	return 1;
}


static void FreeFilter(void *x)
{
	free(x);
}

static void FreeGlob(void *x)
{
	free(x);
}



/****f* LuaListAPI/BindKey
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
int gmel_BindKey(lua_State *L)
{
	struct lstr kstring;
	struct lstr ktitle;
	uint32_t key;
	uListFile *lf;
	uKeyBinding *kb;

	lf = GetListData(L);
	assert(lf != NULL);

	key = luaL_checknumber(L, 1);
	GET_LUA_STRING(ktitle, 2);
	GET_LUA_STRING(kstring, 3);

	kb = ScanKey(lf->lstHotKeys, key);
	if(kb != NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	kb = malloc(sizeof(uKeyBinding));

	kb->key = key;
	kb->sCommand = strdup(kstring.data);
	kb->sTitle = strdup(ktitle.data);

	dlist_ins(lf->lstHotKeys, kb);

	lua_pushnumber(L, 0);
	return 1;
}

/****f* LuaListAPI/QuitViewer
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
int gmel_QuitViewer(lua_State *L)
{
	uListFile *lf;
	lf = GetListData(L);
	assert(lf != NULL);
	lf->quit_flag = 1;

	return 0;
}


int gmel_About(lua_State *L)
{
	uGlobalData *gd;
	uListFile *lf;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lf = GetListData(L);
	assert(lf != NULL);

	about_window(gd);

	lf->redraw = 1;

	return 0;
}


static int DisplayCLI(uListFile *lf)
{
	lf->w->screen->set_style(STYLE_TITLE);
	lf->w->screen->set_cursor(lf->w->screen->get_screen_height()-1, 1);
	lf->w->screen->print(" > ");

	lf->w->screen->set_style(STYLE_NORMAL);
	lf->w->screen->erase_eol();
	lf->w->screen->set_cursor(lf->w->screen->get_screen_height()-1, 4);

	lf->w->screen->print(lf->command);

	lf->w->screen->set_cursor(lf->w->screen->get_screen_height()-1, 4 + strlen(lf->command));

	return 0;
}



static void CalcListDirStats(uGlobalData *gd, uWindow *w, DList *lstFiles)
{
	DLElement *e;
	uDirEntry *de;

	w->total_size = 0;
	w->hidden_count = 0;

	e = dlist_head(lstFiles);
	while (e != NULL)
	{
		de = dlist_data(e);

		if (ALFC_IsHidden(de->name, de->attrs) == 0)
			w->hidden_count += 1;


#ifndef __WIN32__
		// test link to see if its a directory...
		if (S_ISLNK(de->attrs) != 0)
			w->total_size += de->lnk_size;
		else
			w->total_size += de->size;
#else
		w->total_size += de->size;
#endif

		e = dlist_next(e);
	}
}





static int HaveColumnDate(uListFile *lf)
{
	if (strchr(lf->columns, 'd') == NULL)
		return 0;
	else
		return 1;
}

static int HaveColumnSize(uListFile *lf)
{
	if (strchr(lf->columns, 's') == NULL)
		return 0;
	else
		return 1;
}

static int CalcSizeOff(uListFile *lf, int end)
{
	if (HaveColumnSize(lf) == 0)
		return end;

	if (lf->compress_filesize == 0)
		return end - 10;
	else
		return end - 7;
}

static int CalcDateOff(uListFile *lf, int end)
{
	if (HaveColumnDate(lf) == 0)
		return end;

	return(end - (lf->gd->date_fmt_len + lf->gd->time_fmt_len + 2));
}

static void compress_size(char *buff, uint64_t xx)
{
	int round;

	buff[0] = 0;

	if (xx < 1024)
	{
		sprintf(buff, "%7u", (uint32_t) xx);
	}
	else
	{
		round = ((xx * 10) / 1024) % 10;
		xx /= 1024;
		if (xx < 1024)
		{
			sprintf(buff, "%4u.%iK", (uint32_t) xx, round);
		}
		else
		{
			round = ((xx * 10) / 1024) % 10;
			xx /= 1024;
			if (xx < 1024)
			{
				sprintf(buff, "%4u.%iM", (uint32_t) xx, round);
			}
			else
			{
				round = ((xx * 10) / 1024) % 10;
				xx /= 1024;
				if (xx < 1024)
					sprintf(buff, "%4u.%iG", (uint32_t) xx, round);
				else
				{
					sprintf(buff, "%4u.%iT", (uint32_t) (xx / (1024 * 1024)), round);
				}
			}
		}
	}
}

static void PrintFileLine(uListFile *lf, uDirEntry *de, int i, uWindow *win, int max_namelen, int size_off, int date_off)
{
	char *buff;
	char *buff2;
	char *p;
	uint64_t xx;

	int style;

	buff = malloc(1024);
	assert(buff != NULL);

	assert(de != NULL);

	switch (de->type)
	{
		default:
		case FILETYPE_DEFAULT:
			style = STYLE_NORMAL;
			break;
		case FILETYPE_IMAGE:
			style = STYLE_DIR_IMAGE;
			break;
		case FILETYPE_ARCHIVE:
			style = STYLE_DIR_ARCHIVE;
			break;
		case FILETYPE_DOC:
			style = STYLE_DIR_DOCUMENT;
			break;
		case FILETYPE_BACKUP:
			style = STYLE_DIR_BACKUP;
			break;
		case FILETYPE_MEDIA:
			style = STYLE_DIR_MEDIA;
			break;
		case FILETYPE_EXEC:
			style = STYLE_DIR_EXEC;
			break;
	}

	memset(buff, ' ', 1024);

	if (de == NULL)
		return;

	if (de->tagged == 1)
		buff[0] = '+';


#ifndef __WIN32__
	if (de->lnk != NULL)
	{
		buff2 = calloc(1, strlen(de->name) + strlen(de->lnk) + 16);
		p = buff2;

		style = STYLE_DIR_LINK;
		// exec status overrides link status
		if (ALFC_IsExec(de->name, de->attrs) == 0 && ALFC_IsDir(de->attrs) != 0)
		{
			*p++ = '*';
			style = STYLE_DIR_EXEC;
		}
		else
			*p++ = '@';

		strcat(buff2 + 1, de->name);
		strcat(buff2, " -> ");
		strcat(buff2, de->lnk);
	}
	else
#endif
	{
		buff2 = calloc(1, strlen(de->name) + 16);
		p = buff2;

		if (ALFC_IsExec(de->name, de->attrs) == 0 && ALFC_IsDir(de->attrs) != 0)
		{
			*p++ = '*';
			style = STYLE_DIR_EXEC;
		}

		strcat(p, de->name);
	}

	if (strlen(buff2) > max_namelen)
	{
		memmove(buff + 1, buff2, max_namelen);
		memmove(buff + 1 + max_namelen - 3, "...", 3);
	}
	else
		memmove(buff + 1, buff2, strlen(buff2));

	if (HaveColumnSize(lf) == 1)
	{
		if (ALFC_IsDir(de->attrs) != 0)
		{
			if (win->gd->compress_filesize == 0)
			{
				uint64_t xx = de->size;


#ifndef __WIN32__
				if (S_ISLNK(de->attrs&S_IFLNK) != 0)
					xx = de->lnk_size;
#endif

				sprintf(buff + size_off, "%10" PRIu64, xx);
			}
			else
			{
				xx = de->size;
#ifndef __WIN32__
				if (S_ISLNK(de->attrs&S_IFLNK) != 0)
					xx = de->lnk_size;
#endif

				compress_size(buff + size_off, xx);
			}

			p = strchr(buff + size_off, 0x0);
			*p = ' ';
		}
		else
		{
			style = STYLE_DIR_DIR;
#ifndef __WIN32__
			if (S_ISLNK(de->attrs&S_IFLNK) != 0)
			{
				sprintf(buff + size_off, " <D-LNK>");
				p = strchr(buff + size_off, 0x0);
				*p = ' ';
				style = STYLE_DIR_LINK;
			}
			else
#endif
			{
				sprintf(buff + size_off, " <DIR>");
				p = strchr(buff + size_off, 0x0);
				*p = ' ';
			}
		}
	}

	//buff[date_off] = 0;
	if (i == win->highlight_line)
		style = STYLE_HIGHLIGHT;

	win->screen->set_style(style);

	p = buff + (win->width - 12);

	#ifndef __WIN32__
		// do attributes :: "Attr: rwxrwxrwx"
		memmove(p, "---------", 9);

		if ((de->attrs & S_IRUSR) == S_IRUSR)
			p[0] = 'r';
		if ((de->attrs & S_IWUSR) == S_IWUSR)
			p[1] = 'w';
		if ((de->attrs & S_IXUSR) == S_IXUSR)
			p[2] = 'x';

		if ((de->attrs & S_IRGRP) == S_IRGRP)
			p[3] = 'r';
		if ((de->attrs & S_IWGRP) == S_IWGRP)
			p[4] = 'w';
		if ((de->attrs & S_IXGRP) == S_IXGRP)
			p[5] = 'x';

		if ((de->attrs & S_IROTH) == S_IROTH)
			p[6] = 'r';
		if ((de->attrs & S_IWOTH) == S_IWOTH)
			p[7] = 'w';
		if ((de->attrs & S_IXOTH) == S_IXOTH)
			p[8] = 'x';
#else
		memmove(p, "-----", 5);
		if ((de->attrs & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
			p[0] = 'H';
		if ((de->attrs & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
			p[1] = 'S';
		if ((de->attrs & FILE_ATTRIBUTE_COMPRESSED) == FILE_ATTRIBUTE_COMPRESSED)
			p[2] = 'C';
		if ((de->attrs & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
			p[3] = 'A';
		if ((de->attrs & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
			p[4] = 'R';
#endif

	if (HaveColumnDate(lf) == 1)
	{
		char *dst;
		char *dsd;

		dst = GetDateTimeString(win->gd->time_fmt, de->time);
		dsd = GetDateTimeString(win->gd->date_fmt, de->time);

		p = buff + date_off;
		memmove(p, dst, strlen(dst));

		p += strlen(dst) + 1;
		memmove(p, dsd, strlen(dsd));

		p += strlen(dsd);

		free(dsd);
		free(dst);
	}

	buff[win->width - 2] = 0;

	win->screen->set_cursor(2 + i + win->offset_row, 2 + win->offset_col);
	win->screen->print_abs(buff);

	win->screen->set_style(STYLE_NORMAL);

	free(buff2);
	free(buff);
}



static void DrawNoFile(uWindow *w, int style)
{
	char *buff;

	buff = malloc(w->width + 4);

	memset(buff, ' ', w->width);
	buff[w->width - 2] = 0;

	strncpy(buff, ">> NO FILES <<", 14);
	w->screen->set_style(style);

	w->screen->set_cursor(2 + w->offset_row, 2 + w->offset_col);
	w->screen->print(buff);

	free(buff);
}

void DisplayList(uListFile *lf)
{
	int depth;
	char *buff;
	DLElement *e;
	int i;
	char *path;

	int oldstatus;
	int name_len;
	int size_off;
	int date_off;
	uDirEntry *de;

	uWindow *win = lf->w;
	char *dpath = lf->currDir;
	DList *lstFiles = lf->lstFiles;


	buff = malloc(1024);
	assert(buff != NULL);

	assert(lstFiles != NULL);

	win->screen->set_style(STYLE_TITLE);
	win->screen->draw_border(win);

	oldstatus = win->screen->get_updates();

	win->screen->set_updates(0);

	//path = ConvertDirectoryName(dpath);
	path = replace(dpath, '\\', '/');
	if (strlen(path) < win->width - 6)
		sprintf(buff, "[ %s ]", path);
	else
		sprintf(buff, "[ %s ]", path + (strlen(path) - (win->width - 6)));
	free(path);

	win->screen->set_cursor(win->offset_row + 1, win->offset_col + 2);
	win->screen->set_style(STYLE_TITLE);
	win->screen->print(buff);
	win->screen->set_style(STYLE_NORMAL);

	depth = win->height - 2;

	if (depth > dlist_size(lstFiles))
		depth = dlist_size(lstFiles);

	e = dlist_head(lstFiles);
	i = win->top_line;

	while (i > 0 && e != NULL)
	{
		e = dlist_next(e);
		i -= 1;
	}

	i = 0;

	size_off = CalcSizeOff(lf, win->width - OFF_COL);
	date_off = CalcDateOff(lf, size_off);
	name_len = date_off - 2;

	if (e != NULL)
	{
		while (e != NULL && i < depth)
		{
			de = dlist_data(e);
			e = dlist_next(e);

			PrintFileLine(lf, de, i, win, name_len, size_off, date_off);
			i += 1;
		}
	}
	else
	{
		DrawNoFile(win, STYLE_HIGHLIGHT);
		i += 1;
		win->screen->set_style(STYLE_NORMAL);
	}

	memset(buff, ' ', win->width);
	buff[win->width - 2] = 0;

	while (i < win->height - 2)
	{
		win->screen->set_cursor(2 + i + win->offset_row, 2 + win->offset_col);
		win->screen->print(buff);
		i += 1;
	}

	win->screen->set_updates(oldstatus);
	// set cursor to lower corner of screen...
	win->screen->set_cursor(win->screen->get_screen_height(), win->screen->get_screen_width());

	free(buff);
}


void ListDrawAll(uListFile *lf)
{
	lf->w->screen->set_updates(0);
	DisplayList(lf);
	DrawMenuLine(lf->w->screen, lf->lstHotKeys);
	DisplayCLI(lf);
	lf->w->screen->set_updates(1);
}

void UpdateListDir(uListFile *lf, char *set_to_highlight)
{
	lf->gd->screen->set_updates(0);

	lf->w->total_size = 0;
	lf->w->hidden_count = 0;
	lf->w->tagged_count = 0;
	lf->w->tagged_size = 0;

	free(lf->currDir);
	lf->currDir = GetCurrentWorkingDirectory();
	//AddMRU(gd, gd->lstMRULeft, gd->left_dir);

	// get all files
	FreeDList(lf->lstFullFiles);

	SetListFullFiles(lf, GetFiles(lf->currDir));
	CalcListDirStats(lf->gd, lf->w, lf->lstFullFiles);

	// glob + filter em
	FreeDList(lf->lstFiles);
	// applies GLOBAL hidden flag
	SetListFiles(lf, ResetFilteredFileList(lf->gd, lf->lstFilter, lf->lstFullFiles));


	assert(lf->lstGlob != NULL);
	assert(lf->lstFilter != NULL);
	assert(lf->lstFiles != NULL);
	assert(lf->lstFullFiles != NULL);


	UpdateFilterList(lf->gd, lf->lstFilter, lf->lstGlob, lf->lstFullFiles, lf->lstFiles);
	assert(lf->lstFiles != NULL);

	if (set_to_highlight != NULL)
	{
		int idx = GetFileIndex(GetActList(lf->gd), set_to_highlight);
		if (idx >= 0)
			SetHighlightedFile(lf->gd, idx);
	}

	//DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	//DrawActive(gd);
	//DrawFilter(gd);
	//DrawStatusInfoLine(gd);

	lf->gd->screen->set_updates(1);
}


static int go_updir(uListFile *lf)
{
	char cpath[256];
	char *scan;
	char *q;

	getcwd(cpath, 256);

	scan = replace(cpath, '\\', '/');

	q = strchr(scan, 0x0);
	if(q != NULL)
	{
		q -= 1;
		while(*q != '/' && q != scan)
			q -= 1;

		q++;
	}
	else
	{
		q = scan;
	}

	if (chdir("..") != 0)
	{
		free(scan);
		LogInfo("Could not change up directory\n");
		return -1;
	}

	UpdateListDir(lf, q);
	free(scan);

	return 0;
}


static int go_downdir(uListFile *lf)
{
	uDirEntry *de;

	de = GetHighlightedFile(GetActList(lf->gd), lf->w->highlight_line, lf->w->top_line);

	if (ALFC_IsDir(de->attrs) != 0)
		return -1;

	if (chdir(de->name) != 0)
	{
		LogInfo("Could not change to directory %s\n", de->name);
		return -1;
	}

	UpdateListDir(lf, NULL);

	return 0;
}

static int go_scroll_home(uListFile *lf)
{
	lf->w->top_line = 0;
	lf->w->highlight_line = 0;
	DisplayList(lf);

	return 0;
}

static int go_scroll_end(uListFile *lf)
{
	int d = lf->w->height - 2;
	int c = dlist_size(GetActList(lf->gd));
	int tl;

	tl = c - d;

	if (tl < 0)
	{
		lf->w->top_line = 0;
		lf->w->highlight_line = c - 1;
	}
	else
	{
		lf->w->top_line = tl;
		lf->w->highlight_line = d - 1;
	}

	//DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	//DrawActive(gd);

	DisplayList(lf);

	return 0;
}


static int go_scroll_up(uListFile *lf)
{
	uWindow *w;
	int scroll_depth;
	int size_off;
	int max_namelen;
	int date_off;

	w = lf->w;

	if (w->highlight_line == 0 && w->top_line == 0)
	{
		return -1;
	}

	scroll_depth = get_scroll_depth(lf->w);
	size_off = CalcSizeOff(lf, w->width - OFF_COL);
	date_off = CalcDateOff(lf, size_off);
	max_namelen = date_off - 2;

	if ((w->top_line == 0 && w->highlight_line > 0) || (w->highlight_line > 4 && w->top_line > 0))
	{
		uDirEntry *de;

		w->highlight_line -= 1;

		de = GetHighlightedFile(GetActList(lf->gd), w->highlight_line + 1, w->top_line);
		PrintFileLine(lf, de, w->highlight_line + 1, w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), w->highlight_line, w->top_line);
		PrintFileLine(lf, de, w->highlight_line, w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else
	{
		w->top_line -= 1;
		//DrawFileListWindow(w, GetActList(w->gd), GetActDPath(w->gd));
		//DrawActive(gd);
		DisplayList(lf);
		lf->redraw = 1;
	}

	return 0;
}

static int go_scroll_down(uListFile *lf)
{
	uWindow *w;
	int scroll_depth;
	int size_off;
	int date_off;
	int max_namelen;

	w = lf->w;

	if (w->top_line + w->highlight_line + 1 >= dlist_size(GetActList(lf->gd)))
		return -1;

	scroll_depth = get_scroll_depth(w);
	size_off = CalcSizeOff(lf, w->width - OFF_COL);
	date_off = CalcDateOff(lf, size_off);
	max_namelen = date_off - 2;


	// hl is 0 based
	if ((w->highlight_line + 1 < scroll_depth) || (w->top_line + 1 + w->highlight_line >= dlist_size(GetActList(lf->gd)) - 4))
	{
		uDirEntry *de;

		w->highlight_line += 1;

		de = GetHighlightedFile(GetActList(lf->gd), w->highlight_line - 1, w->top_line);
		PrintFileLine(lf, de, w->highlight_line - 1, w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), w->highlight_line, w->top_line);
		PrintFileLine(lf, de, w->highlight_line, w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else
	{
		w->top_line += 1;
		DisplayList(lf);
		lf->redraw = 1;
	}

	return 0;
}

void go_scroll_page_down(uListFile *lf)
{
	int tl;
	int depth;
	int fc;
	int hl;

	uDirEntry *de;
	int size_off;
	int date_off;
	int max_namelen;

	lf->gd->screen->set_updates(0);

	size_off = CalcSizeOff(lf, lf->w->width - OFF_COL);
	date_off = CalcDateOff(lf, size_off);
	max_namelen = date_off - 2;

	tl = lf->w->top_line;
	depth = lf->w->height - 2;
	fc = dlist_size(GetActList(lf->gd));
	hl = lf->w->highlight_line;

	if (dlist_size(GetActList(lf->gd)) < depth)
	{
		// file list is smaller than our window size, so just bottom out.
		depth = dlist_size(GetActList(lf->gd));

		lf->w->highlight_line = depth - 1;

		de = GetHighlightedFile(GetActList(lf->gd), hl, lf->w->top_line);
		PrintFileLine(lf, de, hl, lf->w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), lf->w->highlight_line, lf->w->top_line);
		PrintFileLine(lf, de, lf->w->highlight_line, lf->w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else if (hl < depth - 5)
	{
		// move cursor to bottom of current page
		lf->w->highlight_line = depth - 5;

		de = GetHighlightedFile(GetActList(lf->gd), hl, lf->w->top_line);
		PrintFileLine(lf, de, hl, lf->w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), lf->w->highlight_line, lf->w->top_line);
		PrintFileLine(lf, de, lf->w->highlight_line, lf->w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else
	{
		// scroll actual page
		int c;

		c = fc - tl;
		c -= hl;

		if (c > depth)
		{
			lf->w->top_line += depth - 1;
			DisplayList(lf);
		}
		else
		{
			// whats left is less than an end of page
			go_scroll_end(lf);
		}
	}

	lf->gd->screen->set_updates(1);
}

void go_scroll_page_up(uListFile *lf)
{
	int tl;
	int depth;
	int fc;
	int hl;

	uDirEntry *de;
	int size_off;
	int date_off;
	int max_namelen;

	lf->gd->screen->set_updates(0);

	size_off = CalcSizeOff(lf, lf->w->width - OFF_COL);
	date_off = CalcDateOff(lf, size_off);
	max_namelen = date_off - 2;

	tl = lf->w->top_line;
	depth = lf->w->height - 2;
	fc = dlist_size(GetActList(lf->gd));
	hl = lf->w->highlight_line;

	if (dlist_size(GetActList(lf->gd)) < depth)
	{
		// file list is smaller than our window size, so just top out.
		depth = dlist_size(GetActList(lf->gd));

		if (lf->w->highlight_line == 0)
			return;

		lf->w->highlight_line = 0;

		de = GetHighlightedFile(GetActList(lf->gd), hl, lf->w->top_line);
		PrintFileLine(lf, de, hl, lf->w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), lf->w->highlight_line, lf->w->top_line);
		PrintFileLine(lf, de, lf->w->highlight_line, lf->w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else if (hl > 5)
	{
		// move cursor to top of current page
		lf->w->highlight_line = 5;

		de = GetHighlightedFile(GetActList(lf->gd), hl, lf->w->top_line);
		PrintFileLine(lf, de, hl, lf->w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(lf->gd), lf->w->highlight_line, lf->w->top_line);
		PrintFileLine(lf, de, lf->w->highlight_line, lf->w, max_namelen, size_off, date_off);

		//DrawActiveFileInfo(gd);
	}
	else
	{
		// scroll actual page
		if (tl > depth)
		{
			lf->w->top_line -= depth - 1;
			DisplayList(lf);
		}
		else
		{
			go_scroll_home(lf);
		}
	}

	lf->gd->screen->set_updates(1);
}

int ListFile(uGlobalData *gd)
{
	int rc = 0;
	uListFile *lf;

	char *xdir;
	DList *lstSavedFiles;
	DList *lstSavedFullFiles;
	uWindow *savedWL, *savedWR;

	savedWL = gd->win_left;
	savedWR = gd->win_right;
	lstSavedFiles = gd->lstLeft;
	lstSavedFullFiles = gd->lstFullLeft;

	// save current directory state
	xdir = GetCurrentWorkingDirectory();

	gd->screen->init_list_styles(gd->screen);

	lf = calloc(1, sizeof(uListFile));
	lf->gd = gd;
	lf->w = BuildListWindow(gd);

	// setup to reuse some global funcs
	lf->gd->win_left = lf->w;
	lf->gd->win_right = lf->w;
	lf->gd->selected_window = WINDOW_LEFT;

	lf->columns[0] = 's';
	lf->columns[1] = 'd';

	lf->currDir =  GetCurrentWorkingDirectory();

	lf->lstHotKeys = NewDList(FreeKey);
	lf->lstFilter = NewDList(FreeFilter);
	lf->lstGlob = NewDList(FreeGlob);

	lf->w->highlight_line = 0;
	lf->w->top_line = 0;

	rc = LoadGlobalListScript(lf, INI_get(lf->gd->optfile, "scripts", "list_funcs"));
	if(rc != 0)
		rc = LoadGlobalListScript(lf, "$HOME/.alfc/global.lua");

	gd->screen->set_style(STYLE_NORMAL);
	lf->w->screen->window_clear(lf->w);

	gd->screen->set_style(STYLE_TITLE);
	lf->w->screen->draw_border(lf->w);

	// incase its a HUUUUUUUUUUGE file, throw up a vague title screen
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(1, ((gd->screen->get_screen_width() - (strlen(gstr_WindowTitle) - 8))/2));
	gd->screen->print(gstr_WindowTitle);

	UpdateListDir(lf, NULL);
	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;

	/*

	1 - stat working directory for all files. can we reuse code?
	2 - list!

	*/

	ListDrawAll(lf);

	lf->quit_flag = 0;
	while(lf->quit_flag == 0 && GetQuitAppFlag() == 0)
	{
		uint32_t key;

		key = gd->screen->get_keypress();
		uKeyBinding *kb;
		int mnu;

		lf->w->screen->set_cursor(lf->w->screen->get_screen_height()-1, 4 + strlen(lf->command));
		mnu = ScanMenuOpen(lf->gd, key);

		kb = ScanKey(lf->lstHotKeys, key);
		if(kb != NULL)
		{
			AddHistory(lf->gd, kb->sCommand);

			// exec string via lua...
			if(kb->sCommand[0] == ':')
				exec_internal_list_command(lf, kb->sCommand);
			else if(kb->sCommand[0] == '@')
			{
				char *fn = ConvertDirectoryName(kb->sCommand+1);
				ExecuteListScript(lf, fn);
				free(fn);
			}
			else
			{
				ExecuteGlobalListString(lf, kb->sCommand);
			}

			lf->redraw = 1;
		}
		else
		{
			switch(key)
			{
				case ALFC_KEY_ENTER:
					if(lf->command_length > 0)
					{
						AddHistory(gd, lf->command);

						// exec string via lua...
						if(lf->command[0] == ':')
							exec_internal_list_command(lf, lf->command);
						else if(lf->command[0] == '@')
						{
							char *fn = ConvertDirectoryName(lf->command+1);
							ExecuteListScript(lf, fn);
							free(fn);
						}
						else
						{
							ExecuteListString(lf, lf->command);
						}

						lf->command_length = 0;
						lf->command[lf->command_length] = 0;
						lf->redraw = 1;
					}
					else
					{
						// trigger viewer...
						//char *strNothing = "";
						//CallGlobalFunc(lf->_GL, "ViewFile", "s", strNothing);

						uDirEntry *de;

						de = NULL;
						de = GetHighlightedFile(lf->gd->lstLeft, lf->gd->win_left->highlight_line, lf->gd->win_left->top_line);

						ViewFile(lf->gd, de->name, NULL);

						lf->redraw = 1;
					}
					break;
					
					case 0x21B:	// ESC-ESC
						lf->quit_flag = 1;
						break;

					case ALFC_KEY_LEFT:
						go_updir(lf);
						lf->redraw = 1;
						break;

					case ALFC_KEY_RIGHT:
						go_downdir(lf);
						lf->redraw = 1;
						GetActWindow(gd)->top_line = 0;
						GetActWindow(gd)->highlight_line = 0;
						break;

					case ALFC_KEY_UP:
						go_scroll_up(lf);
						break;

					case ALFC_KEY_DOWN:
						go_scroll_down(lf);
						break;

					case ALFC_KEY_HOME:
						go_scroll_home(lf);
						break;

					case ALFC_KEY_END:
						go_scroll_end(lf);
						break;

					case ALFC_KEY_PAGE_DOWN:
						go_scroll_page_down(lf);
						break;

					case ALFC_KEY_PAGE_UP:
						go_scroll_page_up(lf);
						break;

				default:
					if(key == 0)
						break;

					// terminal could send ^H (0x08) or ASCII DEL (0x7F)
					if ((key == ALFC_KEY_BACKSPACE || key == ALFC_KEY_DEL) || (key >= ' ' && key <= 0x7F))
					{
						if ((key == ALFC_KEY_DEL || key == ALFC_KEY_BACKSPACE))
						{
							if(lf->command_length > 0)
							{
								lf->command_length -= 1;
								lf->command[lf->command_length] = 0;
							}
						}
						else
						{
							if(lf->command_length < lf->w->screen->get_screen_width() - 10)
							{
								lf->command[lf->command_length++] = key;
								lf->command[lf->command_length] = 0;
							}
						}

						DisplayCLI(lf);
					}
					else
						LogInfo("Unknown key 0x%04x\n", key);
					break;

			}

			DisplayCLI(lf);
		}

		if(lf->redraw == 1)
		{
			DisplayList(lf);
			DisplayCLI(lf);
			lf->redraw = 0;
		}
	}

	free(lf->currDir);

	// return to original state
	chdir(xdir);
	free(xdir);

	gd->lstLeft = lstSavedFiles;
	gd->lstFullLeft = lstSavedFullFiles;
	gd->win_left = savedWL;
	gd->win_right = savedWR;

	ReleaseListMenu(lf);

	FreeDList(lf->lstFullFiles);
	dlist_destroy(lf->lstFiles);
	free(lf->lstFiles);

	lf->lstFilter = FreeDList(lf->lstFilter);
	lf->lstGlob = FreeDList(lf->lstGlob);
	lf->lstHotKeys = FreeDList(lf->lstHotKeys);


	if(lf->gcode != NULL)
	{
		free(lf->gcode);
	}

	if(lf->_GL != NULL)
	{
		lua_close(lf->_GL);
	}

	free(lf->w);
	free(lf);

	return 0;
}
