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

	z = strchr(q, '~');
	if(z != NULL)
	{
		strncpy(p, q, z-q);
		strcat(p, "$HOME");
		strcat(p, z + 1);

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

static char* GetOptionDir(uGlobalData *gdata, char *startup, char *group)
{
	char *r;

	r = INI_get(gdata->optfile,"options", "remember_dirs");
	if( IsTrue(r) == 0)
	{
		// yes, so get mru0
		r = INI_get(gdata->optfile, group, "mru0");
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
	DList *lst;
	DIR *d;
	char *cpath;

	lst = malloc(sizeof(DList));
	dlist_init(lst, FreeListEntry);

	cpath = ConvertDirectoryName(path);

	if( chdir(cpath) == 0)
	{
		d = opendir(cpath);
		if(d != NULL)
		{
			struct dirent *dr;
			uDirEntry *de;

			dr = readdir(d);
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

				dlist_ins(lst, de);
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

	return lst;
}

static uint32_t convert_down(off_t a, int count)
{
	off_t x;
	uint32_t z;

	x = a;

	for( ; count > 0; count--)
		//x /= 1024;
		x = x >> 10;

	z = x;

	return z;
}

static void DrawFileListWindow(uWindow *win, DList *lstFiles, char *dpath)
{
	int depth;
	char buff[1024];
	DLElement *e;
	int i;
	char *p;
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
	i = 0;

	if(win->gd->compress_filesize == 0)
		max_sizelen = 10;
	else
		max_sizelen = 7;

	max_namelen = (win->width - max_sizelen - 5);

	while(e != NULL && i < depth)
	{
		uDirEntry *de;
		de = dlist_data(e);
		e = dlist_next(e);

		if(i == win->highlight_line)
		{
			win->screen->set_style(STYLE_HIGHLIGHT);
		}

		win->screen->set_cursor( 2 + i + win->offset_row, 2 + win->offset_col );

		memset(buff, ' ', 1024);

		if(strlen(de->name) > max_namelen)
		{
			memmove(buff, de->name, max_namelen);
			memmove(buff + max_namelen - 3, "...", 3);
		}
		else
			memmove(buff, de->name, strlen(de->name));

		if( S_ISDIR(de->stat_buff.st_mode) == 0 )
		{
			if(win->gd->compress_filesize == 0)
				sprintf(buff + max_namelen + 2, "%10li", de->stat_buff.st_size);
			else
			{
				off_t xx;
				xx = de->stat_buff.st_size;

				xx = convert_down(xx, 1);
				if( xx < 1024)
					sprintf(buff + max_namelen + 2, "%5ikb", (uint32_t)xx);
				else
				{
					xx = convert_down(xx, 1);
					if( xx < 1024)
						sprintf(buff + max_namelen + 2, "%5imb", (uint32_t)xx);
					else
					{
						xx = convert_down(xx, 1);
						if( xx < 1024)
							sprintf(buff + max_namelen + 2, "%5igb", (uint32_t)xx);
						else
						{
							xx = convert_down(xx, 1);
							sprintf(buff + max_namelen + 2, "%5itb", (uint32_t)xx);
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

		win->screen->print(buff);

		if(i == win->highlight_line)
		{
			win->screen->set_style(STYLE_NORMAL);
		}

		i += 1;
	}

	// set cursor to lower corner of screen...
	win->screen->set_cursor(win->screen->get_screen_height(),win->screen->get_screen_width());
}

static char* PrintNumber(off_t num)
{
	char x[32];
	char y[32];
	char *p;
	char *q;
	int i;

	sprintf(x, "%li", num);

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

static void DrawFileInfo(uWindow *win,  DList *lstFiles)
{
	DLElement *e;
	uDirEntry *de;
	char *buff;
	int max_namelen;
	char *siz;

	int size_offset;
	int attr_offset;

	int i;

	i = win->top_line + win->highlight_line;

	e = dlist_head(lstFiles);
	while(e != NULL && i > 0)
	{
		e = dlist_next(e);
		i -= 1;
	}

	// beyond end of file list!
	if(e == NULL || i > 0)
		return;

	de = dlist_data(e);

	win->screen->set_cursor(1 + win->offset_row + win->height, 1);
	win->screen->set_style(STYLE_TITLE);

	buff = malloc(4096);

	// do filename (-20 for file size)

	size_offset = win->screen->get_screen_width() - (20  + 15);
	attr_offset = win->screen->get_screen_width() - (15);

	max_namelen = win->screen->get_screen_width() - (size_offset + 2);
	memset(buff, ' ', 4096);

	sprintf(buff, "File: ");
	if(strlen(de->name) > max_namelen)
	{
		memmove(buff + 6, de->name, max_namelen);
		memmove(buff + 6 + max_namelen - 3, "...", 3);
	}
	else
		memmove(buff + 6, de->name, strlen(de->name));

	if( S_ISDIR(de->stat_buff.st_mode) == 0)
	{
		// do size : "Size: 1,123,123,123"
		memmove(buff + size_offset, "Size: ", 6);
		siz = PrintNumber(de->stat_buff.st_size);
		memmove(buff + size_offset + 6 + (13 - strlen(siz)), siz, strlen(siz));
		free(siz);
	}

	// do attributes :: "Attr: rwxrwxrwx"
	memmove(buff + attr_offset, "Attr: ---------", 15);

#ifndef __MINGW_H
	if( (de->stat_buff.st_mode & S_IRUSR) == S_IRUSR) buff[attr_offset + 6] = 'r';
	if( (de->stat_buff.st_mode & S_IWUSR) == S_IWUSR) buff[attr_offset + 7] = 'w';
	if( (de->stat_buff.st_mode & S_IXUSR) == S_IXUSR) buff[attr_offset + 8] = 'x';

	if( (de->stat_buff.st_mode & S_IRGRP) == S_IRGRP) buff[attr_offset + 9] = 'r';
	if( (de->stat_buff.st_mode & S_IWGRP) == S_IWGRP) buff[attr_offset + 10] = 'w';
	if( (de->stat_buff.st_mode & S_IXGRP) == S_IXGRP) buff[attr_offset + 11] = 'x';

	if( (de->stat_buff.st_mode & S_IROTH) == S_IROTH) buff[attr_offset + 12] = 'r';
	if( (de->stat_buff.st_mode & S_IWOTH) == S_IWOTH) buff[attr_offset + 13] = 'w';
	if( (de->stat_buff.st_mode & S_IWOTH) == S_IWOTH) buff[attr_offset + 14] = 'x';
#else
	if( (de->stat_buff.st_mode & S_IRUSR) == S_IRUSR) buff[attr_offset + 6] = 'r';
	if( (de->stat_buff.st_mode & S_IWUSR) == S_IWUSR) buff[attr_offset + 7] = 'w';
	if( (de->stat_buff.st_mode & S_IXUSR) == S_IXUSR) buff[attr_offset + 8] = 'x';
#endif
	win->screen->print(buff);
	win->screen->set_cursor(win->offset_row + win->height, win->offset_col + 2);
	win->screen->print("[ ** ACTIVE ** ]");

	free(buff);
}

static void DrawActive(uGlobalData *gd)
{
	uWindow *win;
	int i;

	gd->screen->set_style(STYLE_TITLE);

	if(gd->selected_window == WINDOW_LEFT)
		win = gd->win_left;
	else
		win = gd->win_right;

	gd->screen->set_cursor(win->offset_row + win->height, win->offset_col + 2);
	gd->screen->print("[ ** ACTIVE ** ]");

	if(gd->selected_window != WINDOW_LEFT)
		win = gd->win_left;
	else
		win = gd->win_right;


	for(i=0; i<16; i++)
	{
		gd->screen->set_cursor(win->offset_row + win->height, i + win->offset_col + 2);
		gd->screen->print_hline();
	}

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

	gd->win_left->highlight_line = 0; //8 + w->height/2;
	gd->win_right->highlight_line = 8;
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

void SwitchPanes(uGlobalData *gd)
{
	if(gd->selected_window == WINDOW_RIGHT)
	{
		gd->selected_window = WINDOW_LEFT;
		DrawActive(gd);
	}
	else
	{
		gd->selected_window = WINDOW_RIGHT;
		DrawActive(gd);
	}
}

void SetActivePane(uGlobalData *gd, int p)
{
	gd->selected_window = p;
	DrawActive(gd);
}

static void exec_internal_command(char *s)
{
	if(s == NULL)
		return;

	if(strcmp(s, ":quit") == 0)
	{
		SetQuitAppFlag(1);
	}

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

int main(int argc, char *argv[])
{
	uGlobalData *gdata;

#ifdef MEMWATCH
	remove("memwatch.log");
#endif

	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR, 5000);

	gdata = NewGlobalData();

	if(gdata != NULL)
	{
		SetupStartDirectories(gdata);
		LoadOptions(gdata);

		gdata->screen = malloc(sizeof(uScreenDriver));
		memmove(gdata->screen, 	&screen_ncurses, sizeof(uScreenDriver));

		gdata->screen->gd = gdata;
		gdata->screen->init(gdata->screen);

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
		gdata->left_dir = GetOptionDir(gdata, "left_startup", "mru_left");

		free(gdata->right_dir);
		gdata->right_dir = GetOptionDir(gdata, "right_startup", "mru_right");

		LogInfo("Start in left : %s\n", gdata->left_dir);
		LogInfo("Start in right : %s\n", gdata->right_dir);


		gdata->lstLeft = GetFiles(gdata, gdata->left_dir);
		gdata->lstRight = GetFiles(gdata, gdata->right_dir);

		gdata->screen->set_style( STYLE_TITLE);
		gdata->screen->set_cursor(1, ((COLS - (strlen(" Welcome to {A}nother {L}inux {F}ile{M}anager ") - 8))/2));
		gdata->screen->print(" Welcome to {A}nother {L}inux {F}ile{M}anager ");

		DrawFileListWindow(gdata->win_left, gdata->lstLeft, gdata->left_dir);
		DrawFileListWindow(gdata->win_right, gdata->lstRight, gdata->right_dir);

		if(gdata->selected_window == WINDOW_LEFT)
		{
			DrawFileInfo(gdata->win_left, gdata->lstLeft);
			DrawActive(gdata);
		}
		else
		{
			DrawFileInfo(gdata->win_right, gdata->lstRight);
			DrawActive(gdata);
		}

		DrawActive(gdata);

		DrawMenuLine(gdata);
		DrawCLI(gdata);

		while(intFlag == 0)
		{
			uint32_t key;

			key = gdata->screen->get_keypress();

			switch(key)
			{
				// F10
				case F0_KEY + 10:
					intFlag = 1;
					break;

				case TAB_KEY:	// TAB
					SwitchPanes(gdata);
					break;

				case ENTER_KEY:
					if(gdata->command_length > 0)
					{
						// exec string via lua...
						if(gdata->command[0] == ':')
							exec_internal_command(gdata->command);
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

				default:
					// terminal could send ^H (0x08) or ASCII DEL (0x7F)
					if(key == DELETE_KEY || (key >= ' ' && key <= 0x7F))
					{
						if(key == DELETE_KEY)
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

		// cleanup...
		SaveOptions(gdata);
		INI_unload(gdata->optfile);

		gdata->screen->deinit();
		free(gdata->screen);

		chdir(gdata->startup_path);

		dlist_destroy(gdata->lstLeft);
		dlist_destroy(gdata->lstRight);
		free(gdata->lstLeft);
		free(gdata->lstRight);

		free(gdata->win_left);
		free(gdata->win_right);

		free(gdata->startup_path);
		free(gdata->left_dir);
		free(gdata->right_dir);

		free(gdata->optfilename);

		free(gdata->strRealName);
		free(gdata->strLoginName);
		free(gdata->strHomeDirectory);
		free(gdata->strShell);

		free(gdata);

		LogWrite_Shutdown();
	}

	return 0;
}

