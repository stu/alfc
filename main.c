// triggers mingw include
#include <stdlib.h>

#ifdef __MINGW_H
#include <windows.h>
#endif

#ifndef __MINGW_H
#include <pwd.h>
#endif

#include "headers.h"

int intFlag = 0;

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
	/*
	LogInfo("The Real User Name is [%s]\n", gd->strRealName);
	LogInfo("The Login Name is %s\n", gd->strLoginName);
	LogInfo("The Home Directory is %s\n", gd->strHomeDirectory);
	LogInfo("The Login Shell is %s\n", gd->strShell);
	LogInfo("The uid is %lu\n", gd->uid);
	LogInfo("The gid is %lu\n\n", gd->gid);
	*/
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

	r = INI_get(gdata->optfile,"options", "remember_dirs");
	if( IsTrue(r) == 0)
	{
		// yes, so get mru0
		e = dlist_head(lstMRU);
		r = dlist_data(e);
		if(r != NULL)
			return strdup(r);
		else
			return strdup("$HOME");
	}
	else
	{
		r = INI_get(gdata->optfile, "options", startup);
		if(r != NULL)
			return strdup(r);
		else
			return strdup("$HOME");
	}

	return strdup("$HOME");
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
			sprintf(buff + max_namelen + 2, "%10lu", de->size);
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

static void DrawFileListWindow(uWindow *win, DList *lstFiles, char *dpath)
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

	if(e == NULL || i > 0)
		return;

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

static void DrawActive(uGlobalData *gd)
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

static void DrawMenuLine(uGlobalData *gd)
{
	char *buff;
	int m;
	char *p;

	m = gd->screen->get_screen_width();

	buff = malloc(m + 4);

	memset(buff, ' ', m);

	sprintf(buff, "F10 - Quit");

	p = strchr(buff, 0x0);
	*p = ' ';
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
}

void SetActivePane(uGlobalData *gd, int p)
{
	gd->selected_window = p;
	DrawActive(gd);
}

static void exec_internal_command(uGlobalData *gd, char *s)
{
	char *p;

	if(s == NULL)
		return;

	p = strchr(s, ' ');
	if(p == NULL)
		p = strchr(s, 0);
	else
		p -= 1;

	CallGlobalFunc(gd, "CLIParse", "s", s);
}

static void DrawCLI(uGlobalData *gd)
{
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height()-1, 1);
	gd->screen->print(" > ");

	gd->screen->set_style(STYLE_NORMAL);
	gd->screen->erase_eol();
	gd->screen->set_cursor(gd->screen->get_screen_height()-1, 5);

	gd->screen->print(gd->command);
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
		return -1;

	scroll_depth = get_scroll_depth(w);
	max_sizelen = CalcMaxSizeLen(w);
	max_namelen = CalcMaxNameLen(w, max_sizelen);

	if( (w->top_line == 0) || (w->highlight_line > 4 && w->top_line > 0))
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

int updir(uGlobalData *gd)
{
	char *cpath;

	cpath = ConvertDirectoryName(  GetActDPath(gd) );

	if( chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	free(cpath);

	if( chdir("..") != 0)
	{
		LogInfo("Could not change up directory\n");
		return -1;
	}

	if(gd->selected_window == WINDOW_LEFT)
	{
		free(gd->left_dir);
		gd->left_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRULeft, gd->left_dir);
		dlist_destroy(gd->lstLeft);
		free(gd->lstLeft);
		gd->lstLeft = GetFiles(gd, gd->left_dir);
	}
	else
	{
		free(gd->right_dir);
		gd->right_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRURight, gd->right_dir);
		dlist_destroy(gd->lstRight);
		free(gd->lstRight);
		gd->lstRight = GetFiles(gd, gd->right_dir);
	}

	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);

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

	if(gd->selected_window == WINDOW_LEFT)
	{
		free(gd->left_dir);
		gd->left_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRULeft, gd->left_dir);
		dlist_destroy(gd->lstLeft);
		free(gd->lstLeft);
		gd->lstLeft = GetFiles(gd, gd->left_dir);
	}
	else
	{
		free(gd->right_dir);
		gd->right_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRURight, gd->right_dir);
		dlist_destroy(gd->lstRight);
		free(gd->lstRight);
		gd->lstRight = GetFiles(gd, gd->right_dir);
	}

	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;
	GetActWindow(gd)->tagged_count = 0;
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

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

void DrawAll(uGlobalData *gd)
{
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawFileListWindow(GetInActWindow(gd), GetInActList(gd), GetInActDPath(gd));
	DrawActive(gd);

	DrawMenuLine(gd);
	DrawCLI(gd);
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
		SetupStartDirectories(gdata);
		LoadOptions(gdata);

		gdata->screen = malloc(sizeof(uScreenDriver));
		memmove(gdata->screen, 	&screen_ncurses, sizeof(uScreenDriver));

		gdata->screen->gd = gdata;
		gdata->screen->init(gdata->screen);

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

			ExecStartupScript(gdata);

			free(gdata->left_dir);
			gdata->left_dir = GetOptionDir(gdata, "startup_left", gdata->lstMRULeft);

			free(gdata->right_dir);
			gdata->right_dir = GetOptionDir(gdata, "startup_right",  gdata->lstMRURight);

			LogInfo("Start in left : %s\n", gdata->left_dir);
			LogInfo("Start in right : %s\n", gdata->right_dir);


			gdata->lstLeft = GetFiles(gdata, gdata->left_dir);
			gdata->lstRight = GetFiles(gdata, gdata->right_dir);

			gdata->screen->set_style( STYLE_TITLE);
			gdata->screen->set_cursor(1, ((gdata->screen->get_screen_width() - (strlen(" Welcome to {A}nother {L}inux {F}ile{M}anager ") - 8))/2));
			gdata->screen->print(" Welcome to {A}nother {L}inux {F}ile{M}anager ");

			DrawAll(gdata);

			while(intFlag == 0)
			{
				uint32_t key;

				key = gdata->screen->get_keypress();

				switch(key)
				{

					case 0x21B: // ESC-ESC
						intFlag = 1;
						break;

					// F10
					case ALFC_KEY_F10:
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

					case ALFC_KEY_F12:
						tag(gdata);
						DrawStatusInfoLine(gdata);
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

		if(gdata->lstLeft != NULL)
		{
			dlist_destroy(gdata->lstRight);
			free(gdata->lstRight);
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

