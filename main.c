/****h* ALFC/Core
 * FUNCTION
 *   Helper functions for dealing with Lua scripts
 *****
 */

// triggers mingw include
#include <stdlib.h>

#ifdef __MINGW_H
#include <windows.h>
#endif

#ifndef __MINGW_H
#include <pwd.h>
#endif

#include "headers.h"

static void UpdateGlobList(uGlobalData *gd, DList *lstGlob, DList *lstFull, DList *lstF);
static void FreeListEntry(void *x);

int intFlag = 0;


static void SortList(uGlobalData *gd, DList *lstFiles)
{
	CallGlobalFunc(gd, "SortFileList", "");
}


static void FreeFilter(void *x)
{
	free(x);
}

static void FreeGlob(void *x)
{
	free(x);
}

static void FreeKey(void *x)
{
	uKeyBinding *key = x;

	free(key->sTitle);
	free(key->sCommand);
	free(key);
}

// checks an index is in the visible page or not
int IsVisible(uGlobalData *gd, int idx)
{
	int depth;
	uWindow *w = GetActWindow(gd);

	depth = w->height - 2;

	if(idx < w->top_line )
		return 0;

	if( idx > w->top_line + depth )
		return 0;

	return 1;
}

DList* GetActFullList(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->lstFullRight;
	else
		return gd->lstFullLeft;
}

DList* GetActFilter(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->lstFilterRight;
	else
		return gd->lstFilterRight;
}

DList* GetActGlob(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->lstGlobRight;
	else
		return gd->lstGlobLeft;
}

DList* GetActiveMRU(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->lstMRURight;
	else
		return gd->lstMRULeft;
}

static void AddMRU(uGlobalData *gd, DList *lstMRU, char *p)
{
	char *v;
	int c;

	dlist_ins_prev(lstMRU, dlist_head(lstMRU), strdup(p));

	v = INI_get(gd->optfile, "options", "mru_count");
	if(v == NULL)
		c = 16;
	else
		c = atoi(v);

	if(dlist_size(lstMRU) > c)
	{
		dlist_remove(lstMRU, dlist_tail(lstMRU), (void*)&v);
		free(v);
	}
}

char* GetActDPath(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->right_dir;
	else
		return gd->left_dir;
}

char* GetInActDPath(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_LEFT)
		return gd->right_dir;
	else
		return gd->left_dir;
}

DList* GetActList(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->lstRight;
	else
		return gd->lstLeft;
}

DList* GetInActList(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_LEFT)
		return gd->lstRight;
	else
		return gd->lstLeft;
}

uWindow* GetActWindow(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		return gd->win_right;
	else
		return gd->win_left;
}

uWindow* GetInActWindow(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_LEFT)
		return gd->win_right;
	else
		return gd->win_left;
}

void SetQuitAppFlag(int flag)
{
	intFlag = flag;
}

static void GetUserInfo(uGlobalData *gd)
{
#ifdef __MINGW_H
	char UserName[100];
	DWORD UserSize = 100;

	GetUserName(UserName,&UserSize);

	gd->strRealName = strdup(UserName);
	gd->strLoginName = strdup(UserName);
	gd->strHomeDirectory = strdup(getenv("USERPROFILE"));
	gd->strShell = strdup("cmd.exe");
	gd->uid = INT_MAX;
	gd->gid = INT_MAX;
#else
	struct passwd *passwd;
	char *x;
	char *p;

	passwd = getpwuid(getuid());

	x = strdup(passwd->pw_gecos);
	p = strchr(x, 0x0);
	p--;

	while(p>x)
	{
		if(*p == ',' && *(p+1)==0) *p = 0x0;
		if(*p == ' ' && *(p+1)==0) *p = 0x0;

		p--;
	}

	gd->strRealName = strdup(x);
	gd->strLoginName = strdup(passwd->pw_name);
	gd->strHomeDirectory = strdup(passwd->pw_dir);
	gd->strShell = strdup(passwd->pw_shell);
	gd->uid = getpwuid(getuid())->pw_uid;
	gd->gid = getpwuid(getuid())->pw_gid;

	free(x);
#endif
}

static uGlobalData* NewGlobalData(void)
{
	return calloc(1, sizeof(uGlobalData));
}

static char* x_getenv(const char *s)
{
#ifdef __MINGW_H
	if(strcmp(s, "HOME") == 0)
	{
		char *p;

		p = getenv("HOME");
		if(p != NULL)
			return p;

		p = getenv("USERPROFILE");
		if(p != NULL)
			return p;

		return NULL;
	}
	else
#endif
		return getenv(s);
}

/* Convert environment variables to actual strings */
char* ConvertDirectoryName(const char *x)
{
	char *env;

	char *a;
	char *p;
	char *q;
	char *z;

	if(x == NULL)
		return strdup("");

	a = strdup(x);
	p = malloc(1024);
	assert(p != NULL);

	p[0] = 0;
	q = a;

	if(q[0] == '~')
	{
		strcpy(p, "$HOME");
		strcat(p, q+1);

		free(a);
		a = strdup(p);
	}

	p[0] = 0;

	while(*q != 0)
	{
		char *idx;

		env = NULL;
		idx = strchr(q, '$');

		if( idx != NULL)
		{
			char *z;
			char c;

			*idx = 0;

			strcat(p, q);
			q = idx + 1;

			z = strchr(q, '/');
			if(z == NULL)
				z = strchr(q, 0);

			c = *z;
			*z = 0;

			if(p[0] != 0 && p[strlen(p)-1] != '/')
				strcat(p, "/");

			env = x_getenv(q);
			*z = c;

			if(env != NULL)
			{
				if(p[0] != 0 && *env == '/')
					env += 1;

				strcat(p, env);
			}

			q = z;
		}
		else
		{
			strcat(p, q);
			q = strchr(q, 0);
		}
	}


	for(q = p, z = p; *q != 0; )
	{
		switch(*q)
		{
			case '\\':
				*z = '/';
				z++;
				q++;
				break;

			case '"':
				q++;
				break;

			default:
				*z = *q;
				z++;
				q++;
				break;
		}
	}

	*z = 0;

	free(a);
	return realloc(p, 16 + strlen(p));
}


static void ExecStartupScript(uGlobalData *gdata)
{
	char *fn;
	char *cfn;

	fn = INI_get(gdata->optfile, "scripts", "startup_file");
	if(fn != NULL)
	{
		cfn = ConvertDirectoryName(fn);
		ExecuteScript(gdata, cfn);
		free(cfn);
	}
}

static void ExecShutdownScript(uGlobalData *gdata)
{
	char *fn;
	char *cfn;

	fn = INI_get(gdata->optfile, "scripts", "shutdown_file");
	if(fn != NULL)
	{
		cfn = ConvertDirectoryName(fn);
		ExecuteScript(gdata, cfn);
		free(cfn);
	}
}

static char* GetOptionDir(uGlobalData *gdata, char *startup, DList *lstMRU)
{
	char *r;
	DLElement *e;
	char *x;

	r = INI_get(gdata->optfile,"options", "remember_dirs");
	if( IsTrue(r) == 0)
	{
		// yes, so get mru0
		e = dlist_head(lstMRU);
		r = dlist_data(e);
		if(r != NULL)
		{
			x = ConvertDirectoryName(r);
			if( chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			LogInfo("Invalid startup directory %s (%s) does not exist\n", startup, r);
			return GetCurrentWorkingDirectory();
		}
		else
		{
			x = ConvertDirectoryName("$HOME");
			if( chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			LogInfo("Invalid home directory (%s) does not exist\n", x);
			return GetCurrentWorkingDirectory();
		}
	}
	else
	{
		r = INI_get(gdata->optfile, "options", startup);
		if(r != NULL)
		{
			x = ConvertDirectoryName(r);
			if( chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			LogInfo("Invalid startup directory %s (%s) does not exist\n", startup, r);
			return GetCurrentWorkingDirectory();
		}
		else
		{
			x = ConvertDirectoryName("$HOME");
			if( chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			LogInfo("Invalid home directory (%s) does not exist\n", x);
			return GetCurrentWorkingDirectory();
		}
	}

	x = ConvertDirectoryName("$HOME");
	if( chdir(x) == 0)
	{
		free(x);
		return strdup("$HOME");
	}
	LogInfo("Invalid home directory (%s) does not exist\n", x);
	return GetCurrentWorkingDirectory();
}

char* GetCurrentWorkingDirectory(void)
{
	char *x = malloc(4096);
	getcwd(x, 4096);
	return realloc(x, strlen(x) + 1);
}

static void SetupStartDirectories(uGlobalData *gdata)
{
	char *x = GetCurrentWorkingDirectory();

	gdata->left_dir = strdup(x);
	gdata->right_dir = strdup(x);
	gdata->startup_path = strdup(x);

	free(x);
}

static void FreeListEntry(void *x)
{
	struct udtDirEntry *de = x;

	free(de->name);
	free(de);
}

static DList* GetFiles(uGlobalData *gdata, char *path)
{
	DList *lstF;
	DIR *d;
	char *cpath;
	int dirsfirst;
	DLElement *first_file;

	dirsfirst = 0;
	if(IsTrue(INI_get(gdata->optfile, "options", "directories_first"))==0)
		dirsfirst = 1;

	lstF = malloc(sizeof(DList));
	dlist_init(lstF, FreeListEntry);

	cpath = ConvertDirectoryName(path);

	if( chdir(cpath) == 0)
	{
		d = opendir(cpath);
		if(d != NULL)
		{
			struct dirent *dr;
			uDirEntry *de;

			dr = readdir(d);
			first_file = NULL;

			while(dr != NULL)
			{
				if( ( strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0))
				{
					de = malloc(sizeof(uDirEntry));
					memset(de, 0x0, sizeof(uDirEntry));

					de->name = strdup(dr->d_name);
#ifdef __MINGW_H
					stat(de->name, &de->stat_buff);
#else
					lstat(de->name, &de->stat_buff);
#endif

					de->size = de->stat_buff.st_size;
					de->attrs = de->stat_buff.st_mode;

					memset(&de->stat_buff, 0x0, sizeof(struct stat));

					if(dirsfirst == 1 && S_ISDIR(de->attrs) != 0 )
					{
						if(first_file == NULL)
							dlist_ins(lstF, de);
						else
							dlist_ins_prev(lstF, first_file, de);
					}
					else
					{
						dlist_ins(lstF, de);
						if(first_file == NULL)
							first_file = dlist_tail(lstF);
					}
				}
				dr = readdir(d);
			}

			closedir(d);
		}
		else
		{
			LogError("Unable to opendir %s\nerror (%i) %s\n", cpath, errno, strerror(errno));
		}
	}
	else
	{
		LogError("Unable to cd to %s\nerror (%i) %s\n", cpath, errno, strerror(errno));
	}

	free(cpath);

	return lstF;
}

 uint64_t convert_down(uint64_t a, int count)
{
	off_t x;
	uint64_t z;

	x = a;
#ifdef __MING_H
	if(a > INT_MAX)
		a = INT_MAX;
#endif

	for( ; count > 0; count--)
		x = x / 1024;

	z = x;

	return z;
}

static void PrintFileLine(uDirEntry *de, int i, uWindow *win, int max_namelen, int max_sizelen)
{
	char buff[1024];
	char *p;

	if(i == win->highlight_line)
		win->screen->set_style(STYLE_HIGHLIGHT);
	else
		win->screen->set_style(STYLE_NORMAL);

	memset(buff, ' ', 1024);

	if(de == NULL)
		return;


	if(de->tagged == 1)
		buff[0]='+';

	if(strlen(de->name) > max_namelen)
	{
		memmove(buff + 1, de->name, max_namelen);
		memmove(buff + 1 + max_namelen - 3, "...", 3);
	}
	else
		memmove(buff + 1, de->name, strlen(de->name));

	if( S_ISDIR(de->attrs) == 0 )
	{
		if(win->gd->compress_filesize == 0)
#if __WORDSIZE == 64
			sprintf(buff + max_namelen + 2, "%10lu", de->size);
#else
			sprintf(buff + max_namelen + 2, "%10llu", de->size);
#endif
		else
		{
			uint64_t xx = de->size;

			if(xx < 1024)
			{
				sprintf(buff + max_namelen + 2, "%5ub ", (uint32_t)xx);
			}
			else
			{
				xx /= 1024;
				if(xx < 1024)
				{
					sprintf(buff + max_namelen + 2, "%5ukb", (uint32_t)xx);
				}
				else
				{
					xx /= 1024;
					if(xx < 1024)
					{
						sprintf(buff + max_namelen + 2, "%5umb", (uint32_t)xx);
					}
					else
					{
						xx /= 1024;
						if(xx < 1024)
							sprintf(buff + max_namelen + 2, "%5ugb", (uint32_t)xx);
						else
						{
							sprintf(buff + max_namelen + 2, "%5utb", (uint32_t)(xx/(1024*1024)));
						}
					}
				}
			}
		}

		p = strchr(buff + max_namelen + 2, 0x0);
		*p = ' ';
	}
	else
	{
		sprintf(buff + max_namelen + 2, "<DIR>");
		p = strchr(buff + max_namelen + 2, 0x0);
		*p = ' ';
	}


	buff[ win->width - 2] = 0;

	win->screen->set_cursor( 2 + i + win->offset_row, 2 + win->offset_col );
	win->screen->print(buff);

	win->screen->set_style(STYLE_NORMAL);
}

static int CalcMaxSizeLen(uWindow *w)
{
	if(w->gd->compress_filesize == 0)
		return 10;
	else
		return 7;
}

static int CalcMaxNameLen(uWindow *w, int max_size_len)
{
	return (w->width - max_size_len - 5);
}

void DrawFileListWindow(uWindow *win, DList *lstFiles, char *dpath)
{
	int depth;
	char buff[1024];
	DLElement *e;
	int i;
	char *path;

	int max_namelen;
	int max_sizelen;

	win->screen->draw_border(win);

	win->screen->set_style(STYLE_TITLE);

	path = ConvertDirectoryName(dpath);
	if(strlen(path) < win->width-6)
		sprintf(buff, "[ %s ]", path);
	else
		sprintf(buff, "[ %s ]", path + (strlen(path) - (win->width-6)));
	free(path);

	win->screen->set_cursor(win->offset_row + 1, win->offset_col + 2);
	win->screen->print(buff);
	win->screen->set_style(STYLE_NORMAL);

	depth = win->height - 2;

	if(depth > dlist_size(lstFiles))
		depth = dlist_size(lstFiles);

	e = dlist_head(lstFiles);
	i = win->top_line;
	while(i > 0 && e != NULL)
	{
		e = dlist_next(e);
		i -= 1;
	}

	i = 0;
	max_sizelen = CalcMaxSizeLen(win);
	max_namelen = CalcMaxNameLen(win, max_sizelen);

	while(e != NULL && i < depth)
	{
		uDirEntry *de;
		de = dlist_data(e);
		e = dlist_next(e);

		PrintFileLine(de, i, win, max_namelen, max_sizelen);
		i += 1;
	}

	memset(buff, ' ', win->width);
	buff[win->width-2] = 0;

	while(i < win->height - 2)
	{
		win->screen->set_cursor(2 + i + win->offset_row, 2 + win->offset_col );
		win->screen->print(buff);
		i += 1;
	}

	// set cursor to lower corner of screen...
	win->screen->set_cursor(win->screen->get_screen_height(),win->screen->get_screen_width());
}

static char* PrintNumber(uint64_t num)
{
	char x[32];
	char y[32];
	char *p;
	char *q;
	int i;

#if __WORDSIZE == 64
	sprintf(x, "%lu", num);
#else
	sprintf(x, "%llu", num);
#endif

	memset(y, ' ', 32);
	y[31] = 0;

	p = x;
	q = y;
	q += 30;

	p = strchr(x, 0);
	p--;
	i = 0;

	do
	{
		*q-- = *p--;
		i++;
		if(i%3 == 0)
			*q-- = ',';;
	}while(p >= x);

	while(*q == ' ') q++;

	if(*q == ',')
		return strdup(q+1);
	else
		return strdup(q);
}

uDirEntry* GetHighlightedFile(DList *lstFiles, int idx, int tr)
{
	DLElement *e;
	e = dlist_head(lstFiles);
	while(tr > 0 && e != NULL)
	{
		tr -= 1;
		e = dlist_next(e);
	}

	// beyond end of file list!
	if(e == NULL || tr > 0)
		return NULL;

	while(e != NULL && idx > 0)
	{
		e = dlist_next(e);
		idx -= 1;
	}

	// beyond end of file list!
	if(e == NULL || idx > 0)
		return NULL;

	return dlist_data(e);
}

static void DrawFileInfo(uWindow *win)
{
	uDirEntry *de;
	char *buff;
	int max_namelen;
	char *siz;

	int size_offset;
	int attr_offset;

	int i;

	i = win->top_line + win->highlight_line;

	de = GetHighlightedFile(GetActList(win->gd), win->highlight_line, win->top_line);
	win->screen->set_style(STYLE_TITLE);

	buff = malloc(4096);

	// do filename (-20 for file size)

	size_offset = win->screen->get_screen_width() - (20  + 15);
	attr_offset = win->screen->get_screen_width() - (15);
	max_namelen = size_offset - (2 + 6);
	memset(buff, ' ', 4096);

	sprintf(buff, "File: ");
	if(de != NULL)
	{
		if(strlen(de->name) > max_namelen)
		{
			memmove(buff + 6, de->name, max_namelen);
			memmove(buff + 6 + max_namelen - 3, "...", 3);
		}
		else
			memmove(buff + 6, de->name, strlen(de->name));

		// do size : "Size: 1,123,123,123"
		memmove(buff + size_offset, "Size: ", 6);
		if( S_ISDIR(de->attrs) == 0)
		{
			// do size : "Size: 1,123,123,123"
			memmove(buff + size_offset, "Size: ", 6);
			siz = PrintNumber(de->size);
			memmove(buff + size_offset + 6 + (13 - strlen(siz)), siz, strlen(siz));
			free(siz);
		}

		// do attributes :: "Attr: rwxrwxrwx"
		memmove(buff + attr_offset, "Attr: ---------", 15);

#ifndef __MINGW_H
		if( (de->attrs & S_IRUSR) == S_IRUSR) buff[attr_offset + 6] = 'r';
		if( (de->attrs & S_IWUSR) == S_IWUSR) buff[attr_offset + 7] = 'w';
		if( (de->attrs & S_IXUSR) == S_IXUSR) buff[attr_offset + 8] = 'x';

		if( (de->attrs & S_IRGRP) == S_IRGRP) buff[attr_offset + 9] = 'r';
		if( (de->attrs & S_IWGRP) == S_IWGRP) buff[attr_offset + 10] = 'w';
		if( (de->attrs & S_IXGRP) == S_IXGRP) buff[attr_offset + 11] = 'x';

		if( (de->attrs & S_IROTH) == S_IROTH) buff[attr_offset + 12] = 'r';
		if( (de->attrs & S_IWOTH) == S_IWOTH) buff[attr_offset + 13] = 'w';
		if( (de->attrs & S_IWOTH) == S_IWOTH) buff[attr_offset + 14] = 'x';
#else
		if( (de->attrs & S_IRUSR) == S_IRUSR) buff[attr_offset + 6] = 'r';
		if( (de->attrs & S_IWUSR) == S_IWUSR) buff[attr_offset + 7] = 'w';
		if( (de->attrs & S_IXUSR) == S_IXUSR) buff[attr_offset + 8] = 'x';
#endif
	}

	win->screen->set_cursor(1 + win->offset_row + win->height, 1);

	buff[win->screen->get_screen_width()] = 0;
	win->screen->print(buff);
	win->screen->set_style(STYLE_NORMAL);

	free(buff);
}

static void DrawActiveFileInfo(uGlobalData *gd)
{
	DrawFileInfo( GetActWindow(gd));
}

void DrawActive(uGlobalData *gd)
{
	uWindow *win;
	int i;

	gd->screen->set_style(STYLE_TITLE);

	win = GetActWindow(gd);

	gd->screen->set_cursor(win->offset_row + win->height, win->offset_col + 2);
	gd->screen->print("[ ** ACTIVE ** ]");

	win = GetInActWindow(gd);

	for(i=0; i<16; i++)
	{
		gd->screen->set_cursor(win->offset_row + win->height, i + win->offset_col + 2);
		gd->screen->print_hline();
	}

	DrawActiveFileInfo(gd);
	gd->screen->set_style(STYLE_NORMAL);
	gd->screen->set_cursor(gd->screen->get_screen_height(), gd->screen->get_screen_width());
}

static void BuildWindowLayout(uGlobalData *gd)
{
	uWindow *w;

	int mw = gd->screen->get_screen_width() - 0;

	if(gd->win_left == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_left;

	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = 0;
	w->offset_col = (gd->screen->get_screen_width() - mw) / 2;
	w->width = mw/2;
	w->height = gd->screen->get_screen_height() - 5;
	gd->win_left = w;


	if(gd->win_right == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_right;

	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = 0;
	w->offset_col = gd->screen->get_screen_width() / 2;
	w->width = gd->screen->get_screen_width() - gd->win_left->width;
	w->height = gd->screen->get_screen_height() - 5;
	gd->win_right = w;

	gd->win_left->top_line = 0;
	gd->win_right->top_line = 0;
	gd->win_left->highlight_line = 0; //8 + w->height/2;
	gd->win_right->highlight_line = 0;
}

static char* ConvertKeyToName(int key)
{
	struct udtKeyNames
	{
		int key;
		char *name;
	} keys[] =
	{
		{ ALFC_KEY_DOWN, "Down" },
		{ ALFC_KEY_UP, "Up" },
		{ ALFC_KEY_LEFT, "Left" },
		{ ALFC_KEY_RIGHT, "Right" },
		{ ALFC_KEY_INS, "Insert" },
		{ ALFC_KEY_DEL, "Delete" },
		{ ALFC_KEY_BACKSPACE, "Backspace" },
		{ ALFC_KEY_HOME, "Home" },
		{ ALFC_KEY_END, "End" },
		{ ALFC_KEY_PAGE_UP, "Page Up" },
		{ ALFC_KEY_PAGE_DOWN, "Page Down" },
		{ ALFC_KEY_ENTER, "Enter" },
		{ ALFC_KEY_TAB, "Tab" },
		{ ALFC_KEY_SPACE, "Space" },
		{ ALFC_KEY_F00, "F00" },
		{ ALFC_KEY_F01, "F01" },
		{ ALFC_KEY_F02, "F02" },
		{ ALFC_KEY_F03, "F03" },
		{ ALFC_KEY_F04, "F04" },
		{ ALFC_KEY_F05, "F05" },
		{ ALFC_KEY_F06, "F06" },
		{ ALFC_KEY_F07, "F07" },
		{ ALFC_KEY_F08, "F08" },
		{ ALFC_KEY_F09, "F09" },
		{ ALFC_KEY_F10, "F10" },
		{ ALFC_KEY_F11, "F11" },
		{ ALFC_KEY_F12, "F12" },
		{ -1, NULL }
	};

	int j;

	for(j=0; keys[j].key != -1; j++)
	{
		if(keys[j].key == key)
			return keys[j].name;
	}


	return "(UNKNOWN KEY)";
}

static void DrawMenuLine(uGlobalData *gd)
{
	char *buff;
	int m;
	char *q;

	DLElement *e;

	m = gd->screen->get_screen_width();
	buff = malloc(m + 4);
	memset(buff, ' ', m);

	e = dlist_head(gd->lstHotKeys);
	q = buff;

	while(e != NULL)
	{
		uKeyBinding *kb;
		char *keyname;

		kb = dlist_data(e);


		if(e != dlist_head(gd->lstHotKeys))
			q+= 2;

		keyname = ConvertKeyToName(kb->key);
		sprintf(q, "%s - %s", keyname, kb->sTitle);
		q = strchr(buff, 0x0);
		*q = ' ';

		q++;

		e = dlist_next(e);
	}


	buff[m] = 0;
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height(), 1);
	gd->screen->print(buff);

	free(buff);
}

void DrawStatusInfoLine(uGlobalData *gd)
{
	char *buff;
	int m = gd->screen->get_screen_width();
	char *p;

	buff = malloc(4 + m);

	memset(buff, ' ', m);

	sprintf(buff, "Tagged : %i file%c", GetActWindow(gd)->tagged_count, GetActWindow(gd)->tagged_count == 1 ? 0 : 's' );
	p = strchr(buff, 0x0);
	*p = ' ';
	buff[m] = 0;

	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height()-2, 1);
	gd->screen->print(buff);

	free(buff);
}

void SwitchPanes(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
		gd->selected_window = WINDOW_LEFT;
	else
		gd->selected_window = WINDOW_RIGHT;

	DrawActive(gd);
	DrawStatusInfoLine(gd);
	DrawFilter(gd);
}

void SetActivePane(uGlobalData *gd, int p)
{
	gd->selected_window = p;
	DrawActive(gd);
	DrawFilter(gd);
}

/****f* Core/exec_internal_command
* FUNCTION
*	This is where the command line goes to call its lua function
* SYNOPSIS
*/
static void exec_internal_command(uGlobalData *gd, char *command)
/*
* INPUTS
*	o uGlobalData -- standard for all functions
*	o command -- entire string to pass to the CLI global lua function
* RETURNS
*   o None. We dont care or want to know what comes back.
* AUTHOR
*	Stu George
* EXAMPLE
exec_internal_command(gd, ":q");
* SEE ALSO
* 	Lua_Helper/CallGlobalFunc
* NOTES
* 	Strings returned from this function must be free'd.
* SOURCE
*/
{
	if(command == NULL)
		return;

	CallGlobalFunc(gd, "CLIParse", "s", command);
}
/*
*****
*/

static void DrawCLI(uGlobalData *gd)
{
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height()-1, 1);
	gd->screen->print(" > ");

	gd->screen->set_style(STYLE_NORMAL);
	gd->screen->erase_eol();
	gd->screen->set_cursor(gd->screen->get_screen_height()-1, 4);

	gd->screen->print(gd->command);
}

void DrawFilter(uGlobalData *gd)
{
	DLElement *e;
	char *p;
	char *x;

	x = malloc(16 + gd->screen->get_screen_width());

	memset(x, ' ', gd->screen->get_screen_width());
	memmove(x, "Filter: ", 8);

	e = dlist_head(GetActFilter(gd));
	p = x + 8;

	while(e != NULL)
	{
		char *q;

		q = dlist_data(e);
		if(p > (x+8))
		{
			*p++ = ',';
			*p++ = ' ';
		}

		memmove(p, q, strlen(q));
		p += strlen(q);

		e = dlist_next(e);
	}

	e = dlist_head(GetActGlob(gd));
	while(e != NULL)
	{
		char *q;

		q = dlist_data(e);
		if(p > (x+8))
		{
			*p++ = ',';
			*p++ = ' ';
		}

		memmove(p, q, strlen(q));
		p += strlen(q);

		e = dlist_next(e);
	}



	x[gd->screen->get_screen_width()] = 0;

	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height()-3, 1);
	gd->screen->print(x);
	gd->screen->set_style(STYLE_NORMAL);

	free(x);
}


static int get_scroll_depth(uWindow *w)
{
	if( dlist_size(GetActList(w->gd)) < w->height-2)
		return dlist_size(GetActList(w->gd));
	else
		return (w->height - 2) - 4;
}

int scroll_home(uGlobalData *gd)
{
	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);

	return 0;
}

int scroll_end(uGlobalData *gd)
{
	int d = GetActWindow(gd)->height - 2;
	int c = dlist_size(GetActList(gd));
	int tl;

	tl = c - d;

	if(tl < 0)
	{
		GetActWindow(gd)->top_line = 0;
		GetActWindow(gd)->highlight_line = c - 1;
	}
	else
	{
		GetActWindow(gd)->top_line = tl;
		GetActWindow(gd)->highlight_line = d - 1;
	}

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);

	return 0;
}

int scroll_up(uGlobalData *gd)
{
	uWindow *w;
	int scroll_depth;
	int max_sizelen;
	int max_namelen;

	w = GetActWindow(gd);

	if(w->highlight_line == 0 && w->top_line == 0)
	{
		return -1;
	}

	scroll_depth = get_scroll_depth(w);
	max_sizelen = CalcMaxSizeLen(w);
	max_namelen = CalcMaxNameLen(w, max_sizelen);

	if( (w->top_line == 0 && w->highlight_line > 0) || (w->highlight_line > 4 && w->top_line > 0))
	{
		uDirEntry *de;

		w->highlight_line -= 1;

		de = GetHighlightedFile(GetActList(gd), w->highlight_line + 1, w->top_line);
		PrintFileLine(de, w->highlight_line + 1, w, max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
		PrintFileLine(de, w->highlight_line, w, max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);
	}
	else
	{
		w->top_line -= 1;
		DrawFileListWindow(w, GetActList(w->gd), GetActDPath(w->gd));
		DrawActive(gd);
	}

	return 0;
}

int scroll_down(uGlobalData *gd)
{
	uWindow *w;
	int scroll_depth;
	int max_sizelen;
	int max_namelen;

	w = GetActWindow(gd);

	if(w->top_line + w->highlight_line + 1 == dlist_size(GetActList(gd)))
		return -1;

	scroll_depth = get_scroll_depth(w);
	max_sizelen = CalcMaxSizeLen(w);
	max_namelen = CalcMaxNameLen(w, max_sizelen);

	// hl is 0 based
	if( (w->highlight_line + 1 < scroll_depth) || ( w->top_line + 1 + w->highlight_line >= dlist_size(GetActList(gd)) - 4))
	{
		uDirEntry *de;

		w->highlight_line += 1;

		de = GetHighlightedFile(GetActList(gd), w->highlight_line - 1, w->top_line);
		PrintFileLine(de, w->highlight_line - 1, w, max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
		PrintFileLine(de, w->highlight_line, w, max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);
	}
	else
	{
		w->top_line += 1;
		DrawFileListWindow(w, GetActList(w->gd), GetActDPath(w->gd));
		DrawActive(gd);
	}

	return 0;
}

void AddHistory(uGlobalData *gd, char *str, ...)
{
	char *x;

	x = malloc(8192);

	va_list args;
	va_start(args, str);
	vsprintf(x, str, args);
	va_end(args);

	x = realloc(x, strlen(x) + 1);
	dlist_ins(gd->lstLogHistory, x);
}

int GetFileIndex(DList *lstFiles, char *name)
{
	DLElement *d;
	uDirEntry *de;
	int count;

	d = dlist_head(lstFiles);
	count = 0;
	while(d != NULL)
	{
		de = dlist_data(d);

		if( strcmp(de->name, name) == 0)
			return count;

		d = dlist_next(d);
		count += 1;
	}

	return -1;
}

uDirEntry* GetFileByName(DList *lstFiles, char *name)
{
	DLElement *d;
	uDirEntry *de;

	d = dlist_head(lstFiles);
	while(d != NULL)
	{
		de = dlist_data(d);

		if( strcmp(de->name, name) == 0)
			return de;

		d = dlist_next(d);
	}

	return NULL;
}


int SetHighlightedFile(uGlobalData *gd, int idx)
{
	int depth = GetActWindow(gd)->height - 2;

	if(idx != -1)
	{
		int new_top;
		int new_hl;
		int size;

		size = dlist_size(GetActList(gd));
		if(depth > size)
			depth = size;

		if(idx > depth/2)
		{
			new_top = idx - depth/2;
			new_hl = depth - (depth/2) - (depth%2);

			if( idx + depth/2 > size   )
			{
				new_top = size - depth;
				new_hl = idx - new_top;
			}
		}
		else
		{
			new_top = 0;
			new_hl = idx;
		}

		assert(new_top >=0);
		assert(new_hl >= 0);

		GetActWindow(gd)->highlight_line = new_hl;
		GetActWindow(gd)->top_line = new_top;
	}

	return idx;
}


DList* ResetFilteredFileList(DList *lstF, DList *lstA)
{
	DList *lstB;
	DLElement *e;

	lstB = malloc(sizeof(DList));
	dlist_init(lstB, NULL);

	e = dlist_head(lstA);

	while(e != NULL)
	{
		dlist_ins(lstB, dlist_data(e));
		e = dlist_next(e);
	}

	e = dlist_head(lstF);
	while(e != NULL)
	{
		char *x;

		dlist_remove(lstF, e, (void*)&x);
		free(x);

		e = dlist_head(lstF);
	}

	return lstB;
}


static void UpdateDir(uGlobalData *gd, char *set_to_highlight)
{
	if(gd->selected_window == WINDOW_LEFT)
	{
		free(gd->left_dir);
		gd->left_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRULeft, gd->left_dir);
		if(gd->lstFullLeft != NULL)
		{
			dlist_destroy(gd->lstFullLeft);
			free(gd->lstFullLeft);
		}
		gd->lstFullLeft = GetFiles(gd, gd->left_dir);

		if(gd->lstLeft != NULL)
		{
			dlist_destroy(gd->lstLeft);
			free(gd->lstLeft);
		}
		gd->lstLeft = ResetFilteredFileList(gd->lstFilterLeft, gd->lstFullLeft);

		assert(gd->lstFilterLeft != NULL);
		assert(gd->lstFullLeft != NULL);
		assert(gd->lstLeft != NULL);
		assert(gd->lstGlobLeft != NULL);
		UpdateFilterList(gd, gd->lstFilterLeft, gd->lstGlobLeft, gd->lstFullLeft, gd->lstLeft);
	}
	else
	{
		free(gd->right_dir);
		gd->right_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRURight, gd->right_dir);
		if(gd->lstFullRight != NULL)
		{
			dlist_destroy(gd->lstFullRight);
			free(gd->lstFullRight);
		}
		gd->lstFullRight = GetFiles(gd, gd->right_dir);

		if(gd->lstRight != NULL)
		{
			dlist_destroy(gd->lstRight);
			free(gd->lstRight);
		}
		gd->lstRight = ResetFilteredFileList(gd->lstFilterRight, gd->lstFullRight);

		assert(gd->lstFilterRight != NULL);
		assert(gd->lstFullRight != NULL);
		assert(gd->lstRight != NULL);
		assert(gd->lstGlobRight != NULL);
		UpdateFilterList(gd, gd->lstFilterRight, gd->lstGlobRight, gd->lstFullRight, gd->lstRight);
	}

	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;

	if(set_to_highlight != NULL)
	{
		int idx = GetFileIndex(GetActList(gd), set_to_highlight);
		SetHighlightedFile(gd, idx);
	}

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
}

int change_dir(uGlobalData *gd, char *dir)
{
	char *cpath;

	cpath = ConvertDirectoryName( dir );

	if( chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	free(cpath);

	UpdateDir(gd, NULL);

	return 0;
}


int updir(uGlobalData *gd)
{
	char *cpath;
	char *scan;

	cpath = ConvertDirectoryName(  GetActDPath(gd) );

	if( chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	scan = strchr(cpath, 0);
	while(scan > cpath && *scan != '/')
		scan -= 1;

	if(*scan == '/') scan += 1;

	if( chdir("..") != 0)
	{
		free(cpath);
		LogInfo("Could not change up directory\n");
		return -1;
	}

	UpdateDir(gd, scan);
	free(cpath);

	return 0;
}


int godir(uGlobalData *gd, char *dir)
{
	char *cpath;

	cpath = ConvertDirectoryName( GetActDPath(gd) );

	if( chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	free(cpath);

	cpath = ConvertDirectoryName(dir);

	if( chdir(cpath) != 0)
	{
		free(cpath);
		LogInfo("Could not change to directory\n");
		return -1;
	}

	UpdateDir(gd, NULL);
	free(cpath);

	return 0;
}


int downdir(uGlobalData *gd)
{
	char *cpath;
	uDirEntry *de;

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);

	if( S_ISDIR(de->attrs) == 0 )
		return -1;

	cpath = ConvertDirectoryName(  GetActDPath(gd) );

	if( chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	free(cpath);


	if( chdir(de->name) != 0)
	{
		LogInfo("Could not change down to directory %s\n", de->name);
		return -1;
	}

	UpdateDir(gd, NULL);

	return 0;
}

void tag(uGlobalData *gd)
{
	uDirEntry *de;

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
	de->tagged ^= 1;
	if(de->tagged == 1)
		GetActWindow(gd)->tagged_count += 1;
	else
		GetActWindow(gd)->tagged_count -= 1;

	gd->screen->set_style(STYLE_HIGHLIGHT);
	gd->screen->set_cursor(GetActWindow(gd)->offset_row + GetActWindow(gd)->highlight_line + 2, GetActWindow(gd)->offset_col + 2 );
	gd->screen->print( de->tagged == 1 ? "+" : " ");
	gd->screen->set_style(STYLE_NORMAL);

	scroll_down(gd);
}

void scroll_page_down(uGlobalData *gd)
{
	int tl;
	int depth;
	int fc;
	int hl;

	uDirEntry *de;
	int max_sizelen;
	int max_namelen;

	max_sizelen = CalcMaxSizeLen(GetActWindow(gd));
	max_namelen = CalcMaxNameLen(GetActWindow(gd), max_sizelen);

	tl = GetActWindow(gd)->top_line;
	depth = GetActWindow(gd)->height - 2;
	fc = dlist_size(GetActList(gd));
	hl = GetActWindow(gd)->highlight_line;

	if(dlist_size(GetActList(gd)) < depth )
	{
		// file list is smaller than our window size, so just bottom out.
		depth = dlist_size(GetActList(gd));

		GetActWindow(gd)->highlight_line = depth-1;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);
		return;
	}
	else if( hl < depth - 5 )
	{
		// move cursor to bottom of current page
		GetActWindow(gd)->highlight_line = depth-5;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);

		return;
	}
	else
	{
		// scroll actual page
		int c;

		c = fc - tl;
		c -= hl;

		if(c > depth)
		{
			GetActWindow(gd)->top_line += depth - 1;

			DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
			DrawActive(gd);
		}
		else
		{
			// whats left is less than an end of page
			scroll_end(gd);
		}
	}
}


void scroll_page_up(uGlobalData *gd)
{
	int tl;
	int depth;
	int fc;
	int hl;

	uDirEntry *de;
	int max_sizelen;
	int max_namelen;


	max_sizelen = CalcMaxSizeLen(GetActWindow(gd));
	max_namelen = CalcMaxNameLen(GetActWindow(gd), max_sizelen);

	tl = GetActWindow(gd)->top_line;
	depth = GetActWindow(gd)->height - 2;
	fc = dlist_size(GetActList(gd));
	hl = GetActWindow(gd)->highlight_line;

	if(dlist_size(GetActList(gd)) < depth )
	{
		// file list is smaller than our window size, so just top out.
		depth = dlist_size(GetActList(gd));

		if(GetActWindow(gd)->highlight_line == 0)
			return;

		GetActWindow(gd)->highlight_line = 0;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);
		return;
	}
	else if( hl > 5 )
	{
		// move cursor to top of current page
		GetActWindow(gd)->highlight_line = 5;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, max_sizelen);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, max_sizelen);

		DrawActiveFileInfo(gd);

		return;
	}
	else
	{
		// scroll actual page
		if(tl > depth)
		{
			GetActWindow(gd)->top_line -= depth - 1;

			DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
			DrawActive(gd);
		}
		else
		{
			scroll_home(gd);
		}
	}
}

void UpdateFilterList(uGlobalData *gd, DList *lstFilter, DList *lstGlob, DList *lstFull, DList *lstF)
{
	DLElement *e, *e2;
	regex_t preg;
	uDirEntry *de;
	int status;

	DList *lstNew;
	DList *lstOld;

	if(dlist_size(lstFilter) > 0)
	{
		dlist_empty(lstF);

		lstNew = lstFull;
		lstOld = malloc(sizeof(DList));
		dlist_init(lstOld, NULL);

		e = dlist_head(lstFilter);
		while(e != NULL)
		{
			char *pattern;
			int rc;

			pattern = dlist_data(e);
			rc = regcomp(&preg, pattern, REG_EXTENDED|REG_NOSUB);
			if( rc == 0)
			{
				e2 = dlist_head(lstNew);

				while(e2 != NULL)
				{
					de = dlist_data(e2);

					status = regexec(&preg, de->name, 0, NULL, 0);
					if(status == 0)
					{
						dlist_ins(lstF, de);
					}
					else
						dlist_ins(lstOld, de);

					e2 = dlist_next(e2);
				}

				regfree(&preg);
			}
			else
			{
				char buff[128];
				regerror(rc, &preg, buff, 128);
				LogInfo("regex pattern failed to compile\nError : %s\n", buff);
			}

			if(lstNew != lstFull)
			{
				dlist_destroy(lstNew);
				free(lstNew);
			}

			lstNew = lstOld;

			lstOld = malloc(sizeof(DList));
			dlist_init(lstOld, NULL);

			e = dlist_next(e);
		}

		if(lstOld != lstFull)
		{
			dlist_destroy(lstOld);
			free(lstOld);
		}

		if(lstNew != lstFull)
		{
			dlist_destroy(lstNew);
			free(lstNew);
		}
	}

	UpdateGlobList(gd, lstGlob, lstFull, lstF);

	SortList(gd, lstF);
}

static void UpdateGlobList(uGlobalData *gd, DList *lstGlob, DList *lstFull, DList *lstF)
{
	DLElement *e, *e2;
	uDirEntry *de;
	int status;

	DList *lstNew;
	DList *lstOld;

	if(dlist_size(lstGlob) == 0)
		return;

	//dlist_empty(lstF);

	lstNew = lstFull;
	lstOld = malloc(sizeof(DList));
	dlist_init(lstOld, NULL);

	e = dlist_head(lstGlob);
	while(e != NULL)
	{
		char *pattern;

		pattern = dlist_data(e);

		e2 = dlist_head(lstNew);

		while(e2 != NULL)
		{
			de = dlist_data(e2);

			if( S_ISDIR(de->attrs) == 0)
			{
				status = fnmatch(pattern, de->name, 0);
				if(status == 0)
					dlist_ins(lstF, de);
				else
					dlist_ins(lstOld, de);
			}
			else
				dlist_ins(lstF, de);

			e2 = dlist_next(e2);
		}

		if(lstNew != lstFull)
		{
			dlist_destroy(lstNew);
			free(lstNew);
		}

		lstNew = lstOld;

		lstOld = malloc(sizeof(DList));
		dlist_init(lstOld, NULL);

		e = dlist_next(e);
	}

	if(lstOld != lstFull)
	{
		dlist_destroy(lstOld);
		free(lstOld);
	}

	if(lstNew != lstFull)
	{
		dlist_destroy(lstNew);
		free(lstNew);
	}
}


uKeyBinding* ScanKey(uGlobalData *gd, int key)
{
	DLElement *e;
	uKeyBinding *kb;

	e = dlist_head(gd->lstHotKeys);
	while(e != NULL)
	{
		kb = dlist_data(e);

		if(kb->key == key)
		{
			return kb;
		}

		e = dlist_next(e);
	}

	return NULL;
}

void DrawAll(uGlobalData *gd)
{
	int j;

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawFileListWindow(GetInActWindow(gd), GetInActList(gd), GetInActDPath(gd));
	DrawActive(gd);

	gd->screen->set_style(STYLE_TITLE);
	for(j=GetActWindow(gd)->height + 2; j < gd->screen->get_screen_height(); j++ )
	{
		gd->screen->set_cursor(j, 1);
		gd->screen->erase_eol();
	}
	gd->screen->set_style(STYLE_NORMAL);

	DrawMenuLine(gd);
	DrawCLI(gd);
	DrawFilter(gd);
	DrawStatusInfoLine(gd);
}

int main(int argc, char *argv[])
{
	uGlobalData *gdata;

#ifdef MEMWATCH
	remove("memwatch.log");
#endif

	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR | LOG_STDERR, 5000);

	gdata = NewGlobalData();

	if(gdata != NULL)
	{
		gdata->lstHotKeys = malloc(sizeof(DList));
		dlist_init(gdata->lstHotKeys, FreeKey);

		SetupStartDirectories(gdata);
		LoadOptions(gdata);

		gdata->screen = malloc(sizeof(uScreenDriver));
		memmove(gdata->screen, 	&screen_ncurses, sizeof(uScreenDriver));

		gdata->screen->gd = gdata;
		gdata->screen->init(gdata->screen);

		gdata->screen->set_style( STYLE_TITLE);
		gdata->screen->set_cursor(1, ((gdata->screen->get_screen_width() - (strlen(" Welcome to {A}nother {L}inux {F}ile{M}anager ") - 8))/2));
		gdata->screen->print(" Welcome to {A}nother {L}inux {F}ile{M}anager ");


		LogWrite_SetFlags( LogWrite_GetFlags() & ~LOG_STDERR);

		if( LoadGlobalScript(gdata, INI_get(gdata->optfile, "scripts", "global_funcs")) == 0)
		{
			LogInfo("" LUA_RELEASE "; " LUA_COPYRIGHT "\n" LUA_AUTHORS "\n\n");

			if(gdata->screen->get_screen_width() < 60 || gdata->screen->get_screen_height() < 20)
			{
				int r, c;

				c = gdata->screen->get_screen_width();
				r = gdata->screen->get_screen_height();
				gdata->screen->deinit();
				LogError("Display is %i.%i, it must be at least 20x60", r, c);
				exit(1);
			}

			GetUserInfo(gdata);

			BuildWindowLayout(gdata);

			gdata->selected_window = WINDOW_LEFT;
			gdata->lstFilterLeft = malloc(sizeof(DList));
			dlist_init(gdata->lstFilterLeft, FreeFilter);
			gdata->lstGlobLeft = malloc(sizeof(DList));
			dlist_init(gdata->lstGlobLeft, FreeGlob);
			free(gdata->left_dir);
			gdata->left_dir = GetOptionDir(gdata, "startup_left", gdata->lstMRULeft);
			godir(gdata, gdata->left_dir);

			gdata->selected_window = WINDOW_RIGHT;
			gdata->lstFilterRight = malloc(sizeof(DList));
			dlist_init(gdata->lstFilterRight, FreeFilter);
			gdata->lstGlobRight = malloc(sizeof(DList));
			dlist_init(gdata->lstGlobRight, FreeGlob);
			free(gdata->right_dir);
			gdata->right_dir = GetOptionDir(gdata, "startup_right", gdata->lstMRURight);
			godir(gdata, gdata->right_dir);

			gdata->selected_window = WINDOW_LEFT;
			DrawAll(gdata);

			ExecStartupScript(gdata);

			while(intFlag == 0)
			{
				uint32_t key;
				uKeyBinding *kb;

				gdata->screen->set_cursor(gdata->screen->get_screen_height()-1, 4 + strlen(gdata->command));
				key = gdata->screen->get_keypress();

				kb = ScanKey(gdata, key);
				if(kb != NULL)
				{
					// exec string via lua...
					if(kb->sCommand[0] == ':')
						exec_internal_command(gdata, kb->sCommand);
					else if(kb->sCommand[0] == '@')
					{
						char *fn = ConvertDirectoryName(kb->sCommand+1);
						ExecuteScript(gdata, fn);
						free(fn);
					}
					else
					{
						ExecuteString(gdata, kb->sCommand);
					}
				}
				else
				{
					switch(key)
					{

						case 0x21B: // ESC-ESC
							intFlag = 1;
							break;

						case ALFC_KEY_TAB:	// TAB
							SwitchPanes(gdata);
							break;

						case ALFC_KEY_ENTER:
							if(gdata->command_length > 0)
							{
								AddHistory(gdata, gdata->command);

								// exec string via lua...
								if(gdata->command[0] == ':')
									exec_internal_command(gdata, gdata->command);
								else if(gdata->command[0] == '@')
								{
									char *fn = ConvertDirectoryName(gdata->command+1);
									ExecuteScript(gdata, fn);
									free(fn);
								}
								else
								{
									ExecuteString(gdata, gdata->command);
								}

								gdata->command_length = 0;
								gdata->command[gdata->command_length] = 0;

								DrawCLI(gdata);
							}
							break;

						case ALFC_KEY_LEFT:
							updir(gdata);
							break;

						case ALFC_KEY_RIGHT:
							downdir(gdata);
							break;

						case ALFC_KEY_UP:
							scroll_up(gdata);
							break;

						case ALFC_KEY_DOWN:
							scroll_down(gdata);
							break;

						case ALFC_KEY_HOME:
							scroll_home(gdata);
							break;

						case ALFC_KEY_END:
							scroll_end(gdata);
							break;

						case ALFC_KEY_PAGE_DOWN:
							scroll_page_down(gdata);
							break;

						case ALFC_KEY_PAGE_UP:
							scroll_page_up(gdata);
							break;

						default:
							// terminal could send ^H (0x08) or ASCII DEL (0x7F)
							if(key == ALFC_KEY_DEL || (key >= ' ' && key <= 0x7F))
							{
								if(key == ALFC_KEY_DEL)
								{
									if(gdata->command_length > 0)
									{
										gdata->command_length -= 1;
										gdata->command[gdata->command_length] = 0;
									}
								}
								else
								{
									if(gdata->command_length < gdata->screen->get_screen_width() - 10)
									{
										gdata->command[gdata->command_length++] = key;
										gdata->command[gdata->command_length] = 0;
									}
								}

								DrawCLI(gdata);
							}
							else
								LogInfo("Unknown key 0x%04x\n", key);
							break;
					}
				}
			}


			gdata->screen->set_style( STYLE_NORMAL);
			gdata->screen->set_cursor(gdata->screen->get_screen_height(), gdata->screen->get_screen_width());
			gdata->screen->print("\n");
			ExecShutdownScript(gdata);

			SaveMRU(gdata, gdata->lstMRULeft, "mru_left");
			SaveMRU(gdata, gdata->lstMRURight, "mru_right");

			RememberDirectories(gdata);

			// cleanup...
			SaveHistory(gdata);
			SaveOptions(gdata);
		}

		if(gdata->lstHotKeys != NULL)
		{
			dlist_destroy(gdata->lstHotKeys);
			free(gdata->lstHotKeys);
		}

		if(gdata->_GL != NULL)
			lua_close(gdata->_GL);

		if(gdata->gcode != NULL)
			free(gdata->gcode);

		if(gdata->optfile != NULL)
			INI_unload(gdata->optfile);

		if(gdata->screen != NULL)
		{
			gdata->screen->deinit();
			free(gdata->screen);
		}

		if(gdata->startup_path != NULL)
		{
			chdir(gdata->startup_path);
			free(gdata->startup_path);
		}

		if(gdata->lstMRULeft != NULL)
		{
			dlist_destroy(gdata->lstMRULeft);
			free(gdata->lstMRULeft);
		}

		if(gdata->lstMRURight != NULL)
		{
			dlist_destroy(gdata->lstMRURight);
			free(gdata->lstMRURight);
		}

		if(gdata->lstLogHistory != NULL)
		{
			dlist_destroy(gdata->lstLogHistory);
			free(gdata->lstLogHistory);
		}

		if(gdata->lstLeft != NULL)
		{
			dlist_destroy(gdata->lstLeft);
			free(gdata->lstLeft);
		}

		if(gdata->lstRight != NULL)
		{
			dlist_destroy(gdata->lstRight);
			free(gdata->lstRight);
		}

		if(gdata->lstFullLeft != NULL)
		{
			dlist_destroy(gdata->lstFullLeft);
			free(gdata->lstFullLeft);
		}

		if(gdata->lstFullRight != NULL)
		{
			dlist_destroy(gdata->lstFullRight);
			free(gdata->lstFullRight);
		}

		if(gdata->lstFilterLeft != NULL)
		{
			dlist_destroy(gdata->lstFilterLeft);
			free(gdata->lstFilterLeft);
		}

		if(gdata->lstFilterRight != NULL)
		{
			dlist_destroy(gdata->lstFilterRight);
			free(gdata->lstFilterRight);
		}

		if(gdata->lstGlobLeft != NULL)
		{
			dlist_destroy(gdata->lstGlobLeft);
			free(gdata->lstGlobLeft);
		}

		if(gdata->lstGlobRight != NULL)
		{
			dlist_destroy(gdata->lstGlobRight);
			free(gdata->lstGlobRight);
		}



		if(gdata->win_left != NULL)
			free(gdata->win_left);
		if(gdata->win_right != NULL)
			free(gdata->win_right);

		if(gdata->left_dir != NULL)
			free(gdata->left_dir);

		if(gdata->right_dir != NULL)
			free(gdata->right_dir);

		if(gdata->optfilename != NULL)
			free(gdata->optfilename);

		if(gdata->optfilehistory != NULL)
			free(gdata->optfilehistory);

		if(gdata->strRealName != NULL)
			free(gdata->strRealName);
		if(gdata->strLoginName != NULL)
			free(gdata->strLoginName);
		if(gdata->strHomeDirectory != NULL)
			free(gdata->strHomeDirectory);
		if(gdata->strShell != NULL)
			free(gdata->strShell);

		free(gdata);

		LogWrite_Shutdown();
	}

	return 0;
}

