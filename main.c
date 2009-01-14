#include <pwd.h>
#include "headers.h"

static void GetUserInfo(uGlobalData *gd)
{
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

	LogInfo("The Real User Name is [%s]\n", x);
	LogInfo("The Login Name is %s\n", passwd->pw_name);
	LogInfo("The Home Directory is %s\n", passwd->pw_dir);
	LogInfo("The Login Shell is %s\n", passwd->pw_shell);
	LogInfo("The uid is %lu\n", (unsigned long) getpwuid(getuid())->pw_uid);
	LogInfo("The gid is %lu\n\n", (unsigned long) getpwuid(getuid())->pw_gid);

	gd->strRealName = strdup(x);
	gd->strLoginName = strdup(passwd->pw_name);
	gd->strHomeDirectory = strdup(passwd->pw_dir);
	gd->strShell = strdup(passwd->pw_shell);
	gd->uid = getpwuid(getuid())->pw_uid;
	gd->gid = getpwuid(getuid())->pw_gid;

	free(x);
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

static void DrawFileListWindow(uGlobalData *gd, uWindow *win, DList *lstFiles, char *dpath)
{
	int depth;
	char buff[1024];
	DLElement *e;
	int i;
	char *p;
	char *path;

	int max_namelen;
	int max_sizelen;

	gd->screen->draw_border(win);

	gd->screen->set_style(gd, STYLE_TITLE);

	path = ConvertDirectoryName(dpath);
	if(strlen(path) < win->width-6)
		sprintf(buff, "[ %s ]", path);
	else
		sprintf(buff, "[ %s ]", path + (strlen(path) - (win->width-6)));
	free(path);

	gd->screen->set_cursor(win->offset_row + 1, win->offset_col + 2);
	gd->screen->print(buff);
	gd->screen->set_style(gd, STYLE_NORMAL);

	depth = win->height - 2;

	if(depth > dlist_size(lstFiles))
		depth = dlist_size(lstFiles);

	e = dlist_head(lstFiles);
	i = 0;

	win->highlight_line = depth / 2;

	max_sizelen = 10;
	max_namelen = (win->width - max_sizelen - 5);

	while(e != NULL && i < depth)
	{
		uDirEntry *de;
		de = dlist_data(e);
		e = dlist_next(e);

		if(i == win->highlight_line)
		{
				gd->screen->set_style(gd, STYLE_HIGHLIGHT);
		}

		gd->screen->set_cursor( 2 + i + win->offset_row, 2 + win->offset_col );

		memset(buff, ' ', 1024);

		if(strlen(de->name) > max_namelen)
		{
			memmove(buff, de->name, max_namelen);
			memmove(buff + max_namelen - 3, "...", 3);
		}
		else
			memmove(buff, de->name, strlen(de->name));

		sprintf(buff + max_namelen + 2, "%10li", de->stat_buff.st_size);
		p = strchr(buff + max_namelen + 2, 0x0);
		*p = ' ';



		buff[ win->width - 2] = 0;

		gd->screen->print(buff);

		if(i == win->highlight_line)
		{
			gd->screen->set_style(gd, STYLE_NORMAL);
		}

		i += 1;
	}



}

static void BuildWindowLayout(uGlobalData *gd)
{
	uWindow *w;

	int mw = gd->screen->get_screen_width() - 0;

	if(gd->win_left == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_left;

	w->offset_row = 0;
	w->offset_col = (gd->screen->get_screen_width() - mw) / 2;
	w->width = mw/2;
	w->height = gd->screen->get_screen_height() - 5;
	gd->win_left = w;

	if(gd->win_right == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_right;

	w->offset_row = 0;
	w->offset_col = gd->screen->get_screen_width() / 2;
	w->width = gd->screen->get_screen_width() - gd->win_left->width;
	w->height = gd->screen->get_screen_height() - 5;
	gd->win_right = w;
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
		gdata->screen = &screen_ncurses;
		gdata->screen->init(gdata);

		GetUserInfo(gdata);

		LogInfo("" LUA_RELEASE "\n");
		LogInfo("" LUA_COPYRIGHT "\n");
		LogInfo("" LUA_AUTHORS "\n");

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

		gdata->screen->set_style(gdata, STYLE_TITLE);
		gdata->screen->set_cursor(1, ((COLS - (strlen(" Welcome to {A}nother {L}inux {F}ile{M}anager ") - 8))/2));
		gdata->screen->print(" Welcome to {A}nother {L}inux {F}ile{M}anager ");

		DrawFileListWindow(gdata, gdata->win_left, gdata->lstLeft, gdata->left_dir);
		DrawFileListWindow(gdata, gdata->win_right, gdata->lstRight, gdata->right_dir);

		gdata->screen->get_keypress();

		ExecShutdownScript(gdata);

		// cleanup...

		SaveOptions(gdata);
		INI_unload(gdata->optfile);

		gdata->screen->deinit(gdata);

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

