#include "headers.h"

static void UpdateGlobList(uGlobalData *gd, DList *lstGlob, DList *lstFull, DList *lstF);
static void FreeListEntry(void *x);

char *start_left = NULL;
char *start_right = NULL;

int intFlag = 0;

char* gstr_WindowTitle = " Another Linux File Commander ";

void msgbox(uGlobalData *gd, char *str)
{
	uint32_t key;
	uWindow *w;

	char *buff;
	char buff2[128];

	int height;
	int width;
	char *p, *q;


	// big enough to hold string...
	buff = malloc(64 + strlen(str));
	sprintf(buff, "%s\n", str);

	height = 0;
	width = 0;

	p = buff;
	while (*p != 0)
	{
		q = strchr(p, '\n');

		if (q != NULL)
		{
			if (width < q - p)
				width = q - p;
			p = q + 1;
		}
		else
		{
			q = strchr(p, 0);
			if (width < q - p)
				width = q - p;
			p = q;
		}

		height += 1;

	};

	width += 4;
	height += 2;

	w = calloc(1, sizeof(uWindow));
	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = (gd->screen->get_screen_height() - height) / 2;
	w->offset_col = (gd->screen->get_screen_width() - width) / 2;
	w->width = width;
	w->height = height;

	gd->screen->set_style(STYLE_HIGHLIGHT);
	gd->screen->draw_border(w);

	gd->screen->set_style(STYLE_HIGHLIGHT);

	p = buff;
	height = 0;

	while (*p != 0)
	{
		q = strchr(p, '\n');
		if (q != NULL)
			*q = 0;

		memset(buff2, ' ', w->width);
		memmove(buff2 + ((w->width - 2) - strlen(p)) / 2, p, strlen(p));
		buff2[w->width - 2] = 0;

		gd->screen->set_cursor(2 + w->offset_row + height, 2 + w->offset_col);
		gd->screen->print_abs(buff2);

		if (q != NULL)
			p = q + 1;
		else
			p = strchr(p, 0);

		height += 1;
	}

	gd->screen->set_cursor(gd->screen->get_screen_height(), gd->screen->get_screen_width());

	key = gd->screen->get_keypress();

	free(w);
	free(buff);

	gd->screen->set_style(STYLE_TITLE);
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

void ToggleHidden(uGlobalData *gd)
{
	int hidden = IsTrue(INI_get(gd->optfile, "options", "show_hidden"));

	if (hidden == 0)
	{
		INI_UpdateItem(gd->optfile, "options", "show_hidden", "false");
		hidden = 1;
	}
	else
	{
		INI_UpdateItem(gd->optfile, "options", "show_hidden", "true");
		hidden = 0;
	}
}

int GetHiddenFlag(uGlobalData *gd)
{
	int hidden = IsTrue(INI_get(gd->optfile, "options", "show_hidden"));

	return hidden;
}

void SetHiddenFlag(uGlobalData *gd, int hidden)
{
	if (hidden == 0)
	{
		INI_UpdateItem(gd->optfile, "options", "show_hidden", "true");
	}
	else
	{
		INI_UpdateItem(gd->optfile, "options", "show_hidden", "false");
	}
}

static void FreeSubMenu(uSubMenu *m)
{
	if (m->code != NULL)
		free(m->code);
	if (m->name != NULL)
		free(m->name);

	free(m);
}

void FreeMenuData(uMenu *menu[])
{
	int i, j;

	for (i = 0; i < MAX_MENU; i++)
	{
		if (menu[i] != NULL)
		{
			if (menu[i]->name != NULL)
				free(menu[i]->name);

			for (j = 0; j < menu[i]->count; j++)
				FreeSubMenu(menu[i]->child[j]);

			free(menu[i]->child);
			free(menu[i]);

			menu[i] = NULL;
		}
	}
}

static void FreeMenus(uGlobalData *gd)
{
	FreeMenuData(gd->file_menu);
	FreeMenuData(gd->viewer_menu);
	FreeMenuData(gd->list_menu);
}

char* replace(const char *in, char a, char b)
{
	char *x = malloc(strlen(in) + 4);
	char *p = x;

	while (*in != 0)
	{
		*p = *in;

		if (*p == a)
			*p = b;

		p++;
		in++;
	}
	*p++ = *in;

	return x;
}

void FreeFileOp(void *x)
{
	uFileOperation *f = x;

	switch (f->type)
	{
		case eOp_Delete:
			if (f->op.udtDelete.source_filename != NULL)
				free(f->op.udtDelete.source_filename);
			if (f->op.udtDelete.source_path != NULL)
				free(f->op.udtDelete.source_path);
			break;

		case eOp_Move:
			if (f->op.udtMove.source_filename != NULL)
				free(f->op.udtMove.source_filename);
			if (f->op.udtMove.source_path != NULL)
				free(f->op.udtMove.source_path);
			if (f->op.udtMove.dest_filename != NULL)
				free(f->op.udtMove.dest_filename);
			if (f->op.udtMove.dest_path != NULL)
				free(f->op.udtMove.dest_path);
			break;

		case eOp_Copy:
			if (f->op.udtCopy.source_filename != NULL)
				free(f->op.udtCopy.source_filename);
			if (f->op.udtCopy.source_path != NULL)
				free(f->op.udtCopy.source_path);
			if (f->op.udtCopy.dest_filename != NULL)
				free(f->op.udtCopy.dest_filename);
			if (f->op.udtCopy.dest_path != NULL)
				free(f->op.udtCopy.dest_path);
			break;

		case eOp_SymLink:
			if (f->op.udtSymlink.source_filename != NULL)
				free(f->op.udtSymlink.source_filename);
			if (f->op.udtSymlink.source_path != NULL)
				free(f->op.udtSymlink.source_path);
			if (f->op.udtSymlink.dest_filename != NULL)
				free(f->op.udtSymlink.dest_filename);
			if (f->op.udtSymlink.dest_path != NULL)
				free(f->op.udtSymlink.dest_path);
			break;
	}

	if (f->result_msg != NULL)
		free(f->result_msg);

	memset(f, 0x0, sizeof(uFileOperation));
	free(f);
}

char* GetDateTimeString(char *fmt, time_t t)
{
	char *x;
	char *q;
	char *z;
	struct tm *tt;

	char *mns[] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	char *mnl[] =
	{
		"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

	tt = localtime(&t);

	x = calloc(1, 64);
	z = x;

	q = fmt;
	while (*q != 0 && q - fmt < 16 && z - x < 64)
	{
		switch (*q)
		{
			case et_Year4:
				sprintf(z, "%04i", tt->tm_year + 1900);
				break;

			case et_Year2:
				sprintf(z, "%02i", tt->tm_year % 100);
				break;

			case et_Month:
				sprintf(z, "%02i", 1 + tt->tm_mon);
				break;

			case et_Day:
				sprintf(z, "%02i", tt->tm_mday);
				break;

			case et_Hour12:
				sprintf(z, "%02i", (tt->tm_hour % 12) == 0 ? 12 : tt->tm_hour % 12);
				break;

			case et_Hour24:
				sprintf(z, "%02i", tt->tm_hour);
				break;

			case et_Min:
				sprintf(z, "%02i", tt->tm_min);
				break;

			case et_Sec:
				sprintf(z, "%02i", tt->tm_sec);
				break;

			case et_AMPM:
				if (tt->tm_hour < 12)
					sprintf(z, "AM");
				else
					sprintf(z, "PM");
				break;

			case et_MonthNameFull:
				sprintf(z, "%s", mnl[tt->tm_mon]);
				break;

			case et_MonthNameShort:
				sprintf(z, "%s", mns[tt->tm_mon]);
				break;

			default:
				*z++ = *q;
				break;
		}

		q++;
		z = strchr(x, 0);
	}

	if (x[0] == '0' && fmt[0] != et_Year2)
		x[0] = ' ';

	return realloc(x, strlen(x) + 1);
}

void about_window(uGlobalData *gd)
{
	uint32_t key;
	uWindow *w;

	char *buff;
	char buff2[128];

	int height;
	int width;
	char *p, *q;


	// big enough to hold string...
	buff = malloc(1024);
	sprintf(buff, "\n"
			"%s\n"
			"By Stu George\n"
			"\n"
			"Version %i.%02i.%04i\n"
			"Compiled on %s - %s\n"
			"\n"
			"Licensed under the GNU GPL v2\n"
			"\n", gstr_WindowTitle, VersionMajor(), VersionMinor(), VersionBuild(), VersionDate(), VersionTime());

	height = 0;
	width = 0;

	p = buff;
	while (*p != 0)
	{
		q = strchr(p, '\n');

		if (q != NULL)
		{
			if (width < q - p)
				width = q - p;
			p = q + 1;
		}
		else
		{
			q = strchr(p, 0);
			if (width < q - p)
				width = q - p;
			p = q;
		}

		height += 1;

	};

	width += 4;
	height += 2;

	w = calloc(1, sizeof(uWindow));
	w->gd = gd;
	w->screen = gd->screen;

	w->offset_row = (gd->screen->get_screen_height() - height) / 2;
	w->offset_col = (gd->screen->get_screen_width() - width) / 2;
	w->width = width;
	w->height = height;

	gd->screen->set_style(STYLE_HIGHLIGHT);
	gd->screen->draw_border(w);

	gd->screen->set_style(STYLE_HIGHLIGHT);

	p = buff;
	height = 0;

	while (*p != 0)
	{
		q = strchr(p, '\n');
		if (q != NULL)
			*q = 0;

		memset(buff2, ' ', w->width);
		memmove(buff2 + ((w->width - 2) - strlen(p)) / 2, p, strlen(p));
		buff2[w->width - 2] = 0;

		gd->screen->set_cursor(2 + w->offset_row + height, 2 + w->offset_col);
		gd->screen->print_abs(buff2);

		if (q != NULL)
			p = q + 1;
		else
			p = strchr(p, 0);

		height += 1;
	}

	gd->screen->set_cursor(gd->screen->get_screen_height(), gd->screen->get_screen_width());

	key = gd->screen->get_keypress();

	free(w);
	free(buff);

	gd->screen->set_style(STYLE_TITLE);
}

static void SortList(uGlobalData *gd, DList *lstFiles)
{
	CallGlobalFunc(gd->_GL, "SortFileList", "");
}

static void FreeFilter(void *x)
{
	free(x);
}

static void FreeGlob(void *x)
{
	free(x);
}

void FreeKey(void *x)
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

	if (idx < w->top_line)
		return 0;

	if (idx > w->top_line + depth)
		return 0;

	return 1;
}

DList* GetActFullList(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->lstFullRight;
	else
		return gd->lstFullLeft;
}

DList* GetActFilter(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->lstFilterRight;
	else
		return gd->lstFilterRight;
}

DList* GetActGlob(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->lstGlobRight;
	else
		return gd->lstGlobLeft;
}

DList* GetActiveMRU(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->lstMRURight;
	else
		return gd->lstMRULeft;
}

uMenu** GetActMenu(uGlobalData *gd)
{
	if (gd->mode == eMode_Directory)
		return gd->file_menu;
	else if (gd->mode == eMode_Viewer)
		return gd->viewer_menu;
	else if (gd->mode == eMode_VB_List)
		return gd->list_menu;
	else
		LogInfo("UNKNOWN MENU\n");

	return NULL;
}

static void AddMRU(uGlobalData *gd, DList *lstMRU, char *p)
{
	char *v;
	int c;

	if (dlist_size(lstMRU) > 0)
	{
		v = dlist_data(dlist_head(lstMRU));
		if (strcmp(v, p) == 0)
			return;
	}

	dlist_ins_prev(lstMRU, dlist_head(lstMRU), strdup(p));

	v = INI_get(gd->optfile, "options", "mru_count");
	if (v == NULL)
		c = 16;
	else
		c = atoi(v);

	if (c < 0)
		c = 4;

	if (dlist_size(lstMRU) > c)
	{
		dlist_remove(lstMRU, dlist_tail(lstMRU), (void*) &v);
		free(v);
	}
}

char* GetActDPath(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->right_dir;
	else
		return gd->left_dir;
}

char* GetInActDPath(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_LEFT)
		return gd->right_dir;
	else
		return gd->left_dir;
}

DList* GetActList(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->lstRight;
	else
		return gd->lstLeft;
}

DList* GetInActList(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_LEFT)
		return gd->lstRight;
	else
		return gd->lstLeft;
}

uWindow* GetActWindow(uGlobalData *gd)
{
	assert(gd->win_left != NULL);
	assert(gd->win_right != NULL);
	if (gd->selected_window == WINDOW_RIGHT)
		return gd->win_right;
	else
		return gd->win_left;
}

uWindow* GetInActWindow(uGlobalData *gd)
{
	if (gd->selected_window == WINDOW_LEFT)
		return gd->win_right;
	else
		return gd->win_left;
}

uint32_t fletcher32(uint16_t *data, size_t len)
{
	uint32_t sum1 = 0xffff, sum2 = 0xffff;

	while (len)
	{
		unsigned tlen = len > 360 ? 360 : len;
		len -= tlen;

		do
		{
			sum1 += *data++;
			sum2 += sum1;
		} while (--tlen);

		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}

	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);

	return sum2 << 16 | sum1;
}

void SetQuitAppFlag(int flag)
{
	intFlag = flag;
}

int GetQuitAppFlag(void)
{
	return intFlag;
}

static uGlobalData* NewGlobalData(void)
{
	return calloc(1, sizeof(uGlobalData));
}

/* Convert environment variables to actual strings */
char* ConvertDirectoryName(const char *x)
{
	char *env;

	char *a;
	char *p;
	char *q;
	char *z;

	if (x == NULL)
		return strdup("");

	a = strdup(x);
	p = malloc(1024);
	assert(p != NULL);

	p[0] = 0;
	q = a;

	if (q[0] == '~')
	{
		strcpy(p, "$HOME");
		strcat(p, q + 1);

		free(a);
		a = strdup(p);
	}

	p[0] = 0;

	while (*q != 0)
	{
		char *idx;

		env = NULL;
		idx = strchr(q, '$');

		if (idx != NULL)
		{
			char *z;
			char c;

			*idx = 0;

			strcat(p, q);
			q = idx + 1;

			z = strchr(q, '/');
			if (z == NULL)
				z = strchr(q, 0);

			c = *z;
			*z = 0;

			env = ALFC_getenv(q);
			*z = c;

			if (env != NULL)
			{
				if (p[0] != 0 && p[strlen(p) - 1] != '/')
					strcat(p, "/");

				if (p[0] != 0 && *env == '/')
					env += 1;

				strcat(p, env);
				q = z;
			}
			else
			{
				strcat(p, "$");
				q = idx + 1;
			}
		}
		else
		{
			strcat(p, q);
			q = strchr(q, 0);
		}
	}

	for (q = p, z = p; *q != 0;)
	{
		switch (*q)
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
	if (fn != NULL)
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
	if (fn != NULL)
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

	r = INI_get(gdata->optfile, "options", "remember_dirs");
	if (IsTrue(r) == 0)
	{
		// yes, so get mru0
		e = dlist_head(lstMRU);
		r = dlist_data(e);
		if (r != NULL)
		{
			x = ConvertDirectoryName(r);
			if (chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			free(x);
			LogInfo("Invalid startup directory %s (%s) does not exist\n", startup, r);
			return GetCurrentWorkingDirectory();
		}
		else
		{
			x = ConvertDirectoryName("$HOME");
			if (chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			free(x);
			LogInfo("Invalid home directory (%s) does not exist\n", x);
			return GetCurrentWorkingDirectory();
		}
	}
	else
	{
		r = INI_get(gdata->optfile, "options", startup);
		if (r != NULL)
		{
			x = ConvertDirectoryName(r);
			if (chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			free(x);
			LogInfo("x2Invalid startup directory %s (%s) does not exist\n", startup, r);
			return GetCurrentWorkingDirectory();
		}
		else
		{
			x = ConvertDirectoryName("$HOME");
			if (chdir(x) == 0)
			{
				free(x);
				return strdup(r);
			}
			free(x);
			LogInfo("Invalid home directory (%s) does not exist\n", x);
			return GetCurrentWorkingDirectory();
		}
	}

	x = ConvertDirectoryName("$HOME");
	if (chdir(x) == 0)
	{
		free(x);
		return strdup("$HOME");
	}
	free(x);
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
	uDirEntry *de = x;

	if (de->lnk != NULL)
		free(de->lnk);

	free(de->name);
	free(de->path);

	memset(de, 0x0, sizeof(uDirEntry));
	free(de);
}

#ifndef __WIN32__
static char* ReadLink(char *fn, uint32_t len)
{
	char *buff;

	buff = malloc(len + 64);
	memset(buff, 0x0, len + 64);

	readlink(fn, buff, len + 63);

	return realloc(buff, strlen(buff)+1);
}
#endif

void CalcDirStats(uGlobalData *gd, uWindow *w, DList *lstFiles)
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

DList* GetFiles(char *path)
{
	DList *lstF;
	DIR *d;
	char *cpath;
	struct dirent *dr;
	uDirEntry *de;
	struct stat buff;

	lstF = malloc(sizeof(DList));
	dlist_init(lstF, FreeListEntry);

	cpath = replace(path, '\\', '/');

	//assert(gd != NULL);
	//assert(GetActWindow(gd) != NULL);

	d = opendir(cpath);
	if (d != NULL)
	{
		dr = readdir(d);

		while (dr != NULL)
		{
			if ((strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0))
			{
				char *fname;

				fname = malloc(strlen(cpath) + strlen(dr->d_name) + 16);
				strcpy(fname, cpath);
				strcat(fname, "/");
				strcat(fname, dr->d_name);

				ALFC_stat(fname, &buff);

				de = malloc(sizeof(uDirEntry));
				memset(de, 0x0, sizeof(uDirEntry));


				// setup the defaults
				de->attrs = buff.st_mode;
				de->size = buff.st_size;
				de->time = buff.st_mtime;

				de->path = strdup(cpath);
				de->name = strdup(dr->d_name);
				de->attrs = ALFC_GetFileAttrs(de);
				de->size = ALFC_GetFileSize(de);
				de->time = ALFC_GetFileTime(de);

				free(fname);


#ifndef __WIN32__
				// test link to see if its a directory...
				if (S_ISLNK(buff.st_mode) != 0)
				{
					de->lnk = ReadLink(de->name, de->size);

					ALFC_stat(de->lnk, &buff);
					de->lnk_size = ALFC_GetFileSize(de);

					if (S_ISDIR(buff.st_mode) != 0)
					{
						de->attrs |= S_IFDIR;
					}
				}
#endif
				dlist_ins(lstF, de);
			}

			dr = readdir(d);
		}

		closedir(d);
	}
	else
	{
		LogError("Unable to opendir %s\nerror (%i) %s\n", cpath, errno, strerror(errno));
	}

	free(cpath);

	return lstF;
}

static int HaveColumnDate(uGlobalData *gd)
{
	if (strchr(gd->columns, 'd') == NULL)
		return 0;
	else
		return 1;
}

static int HaveColumnSize(uGlobalData *gd)
{
	if (strchr(gd->columns, 's') == NULL)
		return 0;
	else
		return 1;
}

static int CalcSizeOff(uWindow *w, int end)
{
	if (HaveColumnSize(w->gd) == 0)
		return end;

	if (w->gd->compress_filesize == 0)
		return end - 10;
	else
		return end - 7;
}

static int CalcDateOff(uWindow *w, int end)
{
	if (HaveColumnDate(w->gd) == 0)
		return end;

	return(end - (w->gd->date_fmt_len + w->gd->time_fmt_len + 2));
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

static void PrintFileLine(uDirEntry *de, int i, uWindow *win, int max_namelen, int size_off, int date_off)
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

	if (HaveColumnSize(win->gd) == 1)
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

	if (HaveColumnDate(win->gd) == 1)
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

void DrawFileListWindow(uWindow *win, DList *lstFiles, char *dpath)
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

	size_off = CalcSizeOff(win, win->width - 3);
	date_off = CalcDateOff(win, size_off);
	name_len = date_off - 2;

	if (e != NULL)
	{
		while (e != NULL && i < depth)
		{
			de = dlist_data(e);
			e = dlist_next(e);

			PrintFileLine(de, i, win, name_len, size_off, date_off);
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

static char* PrintNumber(uint64_t num)
{
	char x[32];
	char y[32];
	char *p;
	char *q;
	int i;

	sprintf(x, "%" PRIu64, num);

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
		if (i % 3 == 0)
			*q-- = ',';
		;
	} while (p >= x);

	while (*q == ' ')
		q++;

	if (*q == ',')
		return strdup(q + 1);
	else
		return strdup(q);
}

uDirEntry* GetHighlightedFile(DList *lstFiles, int idx, int tr)
{
	DLElement *e;
	e = dlist_head(lstFiles);

	while (tr > 0 && e != NULL)
	{
		tr -= 1;
		e = dlist_next(e);
	}

	// beyond end of file list!
	if (e == NULL || tr > 0)
		return NULL;

	while (e != NULL && idx > 0)
	{
		e = dlist_next(e);
		idx -= 1;
	}

	// beyond end of file list!
	if (e == NULL || idx > 0)
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

	size_offset = win->screen->get_screen_width() - (20 + 15);
	attr_offset = win->screen->get_screen_width() - (15);
	max_namelen = size_offset - (2 + 6);
	memset(buff, ' ', 4096);

	memmove(buff, "File: ", 6);
	if (de != NULL)
	{
		char *buff2;

		if (de->lnk != NULL)
		{
			buff2 = malloc(strlen(de->name) + strlen(de->lnk) + 16);
			strcpy(buff2, de->name);
			strcat(buff2, " -> ");
			strcat(buff2, de->lnk);
		}
		else
		{
			buff2 = malloc(strlen(de->name) + 16);
			strcpy(buff2, de->name);
		}

		if (strlen(buff2) > max_namelen)
		{
			memmove(buff + 6, buff2, max_namelen);
			memmove(buff + 6 + max_namelen - 3, "...", 3);
		}
		else
			memmove(buff + 6, buff2, strlen(buff2));

		free(buff2);


		// do size : "Size: 1,123,123,123"
		memmove(buff + size_offset, "Size: ", 6);
		if (ALFC_IsDir(de->attrs) != 0)
		{
			// do size : "Size: 1,123,123,123"
			//memmove(buff + size_offset, "Size: ", 6);
			siz = PrintNumber(de->size);
			memmove(buff + size_offset + 6 + (13 - strlen(siz)), siz, strlen(siz));
			free(siz);
		}

#ifndef __WIN32__
		// do attributes :: "Attr: rwxrwxrwx"
		memmove(buff + attr_offset, "Attr: ---------", 15);

		if ((de->attrs & S_IRUSR) == S_IRUSR)
			buff[attr_offset + 6] = 'r';
		if ((de->attrs & S_IWUSR) == S_IWUSR)
			buff[attr_offset + 7] = 'w';
		if ((de->attrs & S_IXUSR) == S_IXUSR)
			buff[attr_offset + 8] = 'x';

		if ((de->attrs & S_IRGRP) == S_IRGRP)
			buff[attr_offset + 9] = 'r';
		if ((de->attrs & S_IWGRP) == S_IWGRP)
			buff[attr_offset + 10] = 'w';
		if ((de->attrs & S_IXGRP) == S_IXGRP)
			buff[attr_offset + 11] = 'x';

		if ((de->attrs & S_IROTH) == S_IROTH)
			buff[attr_offset + 12] = 'r';
		if ((de->attrs & S_IWOTH) == S_IWOTH)
			buff[attr_offset + 13] = 'w';
		if ((de->attrs & S_IXOTH) == S_IXOTH)
			buff[attr_offset + 14] = 'x';
#else
		memmove(buff + attr_offset, "Attr: -----", 11);
		if ((de->attrs & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
			buff[attr_offset + 6] = 'H';
		if ((de->attrs & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
			buff[attr_offset + 7] = 'S';
		if ((de->attrs & FILE_ATTRIBUTE_COMPRESSED) == FILE_ATTRIBUTE_COMPRESSED)
			buff[attr_offset + 8] = 'C';
		if ((de->attrs & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
			buff[attr_offset + 9] = 'A';
		if ((de->attrs & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
			buff[attr_offset + 10] = 'R';
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
	DrawFileInfo(GetActWindow(gd));
}

void DrawActive(uGlobalData *gd)
{
	uWindow *win;
	char *sort_opt;
	char buff[256];

	int i;

	gd->screen->set_style(STYLE_TITLE);

	win = GetActWindow(gd);

	sort_opt = INI_get(gd->optfile, "options", "sort_order");

	sprintf(buff, "[ ** ACTIVE ** : Sort: %s ]", sort_opt);

	gd->screen->set_cursor(win->offset_row + win->height, win->offset_col + 2);
	//gd->screen->print("[ ** ACTIVE ** ]");
	gd->screen->print(buff);

	win = GetInActWindow(gd);

	for (i = 0; i < win->width - 2; i++)
	{
		gd->screen->set_cursor(win->offset_row + win->height, i + win->offset_col + 2);
		gd->screen->print_hline();
	}

	DrawActiveFileInfo(gd);
	gd->screen->set_style(STYLE_NORMAL);
	gd->screen->set_cursor(gd->screen->get_screen_height(), gd->screen->get_screen_width());
}

static void BuildWindowLayoutLeft(uWindow *w, char* set_to_highlight)
{
	int oact;

	int mw = w->gd->screen->get_screen_width() - 0;

	oact = w->gd->selected_window;
	w->gd->selected_window = WINDOW_LEFT;

	w->offset_row = 0;
	w->offset_col = (w->gd->screen->get_screen_width() - mw) / 2;
	w->width = mw / 2;
	w->height = w->gd->screen->get_screen_height() - 5;

	w->top_line = 0;
	w->highlight_line = 0;

	if (set_to_highlight)
	{
		int idx = GetFileIndex(GetActList(w->gd), set_to_highlight);
		if (idx >= 0)
			SetHighlightedFile(w->gd, idx);
	}

	w->gd->selected_window = oact;
}

static void BuildWindowLayoutRight(uWindow *w, char* set_to_highlight)
{
	int oact;

	oact = w->gd->selected_window;
	w->gd->selected_window = WINDOW_RIGHT;

	w->offset_row = 0;
	w->offset_col = w->gd->screen->get_screen_width() / 2;
	w->width = w->gd->screen->get_screen_width() - w->gd->win_left->width;
	w->height = w->gd->screen->get_screen_height() - 5;

	w->top_line = 0;
	w->highlight_line = 0;

	if (set_to_highlight != NULL)
	{
		int idx = GetFileIndex(GetActList(w->gd), set_to_highlight);
		if (idx >= 0)
			SetHighlightedFile(w->gd, idx);
	}

	w->gd->selected_window = oact;
}

static void BuildWindowLayout(uGlobalData *gd)
{
	uWindow *w;

	if (gd->win_left == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_left;

	w->gd = gd;
	w->screen = gd->screen;
	BuildWindowLayoutLeft(w, NULL);
	gd->win_left = w;

	if (gd->win_right == NULL)
		w = calloc(1, sizeof(uWindow));
	else
		w = gd->win_right;

	w->gd = gd;
	w->screen = gd->screen;

	BuildWindowLayoutRight(w, NULL);
	gd->win_right = w;
}

char* ConvertKeyToName(int key)
{
	char *s;

	struct udtKeyNames
	{
		int key;
		char *name;
	} keys[] =
	{
		{
			ALFC_KEY_DOWN, "Down"
		},
		{
			ALFC_KEY_UP, "Up"
		},
		{
			ALFC_KEY_LEFT, "Left"
		},
		{
			ALFC_KEY_RIGHT, "Right"
		},
		{
			ALFC_KEY_INS, "Insert"
		},
		{
			ALFC_KEY_DEL, "Delete"
		},
		{
			ALFC_KEY_BACKSPACE, "Backspace"
		},
		{
			ALFC_KEY_HOME, "Home"
		},
		{
			ALFC_KEY_END, "End"
		},
		{
			ALFC_KEY_PAGE_UP, "Page Up"
		},
		{
			ALFC_KEY_PAGE_DOWN, "Page Down"
		},
		{
			ALFC_KEY_ENTER, "Enter"
		},
		{
			ALFC_KEY_TAB, "Tab"
		},
		{
			ALFC_KEY_SPACE, "Space"
		},
		{
			ALFC_KEY_F01, "F1"
		},
		{
			ALFC_KEY_F02, "F2"
		},
		{
			ALFC_KEY_F03, "F3"
		},
		{
			ALFC_KEY_F04, "F4"
		},
		{
			ALFC_KEY_F05, "F5"
		},
		{
			ALFC_KEY_F06, "F6"
		},
		{
			ALFC_KEY_F07, "F7"
		},
		{
			ALFC_KEY_F08, "F8"
		},
		{
			ALFC_KEY_F09, "F9"
		},
		{
			ALFC_KEY_F10, "F10"
		},
		{
			ALFC_KEY_F11, "F11"
		},
		{
			ALFC_KEY_F12, "F12"
		},

		{
			-1, NULL
		}
	};

	int j;

	if (key >= ALFC_KEY_ALT + 'A' && key <= ALFC_KEY_ALT + 'Z')
	{
		s = malloc(32);
		sprintf(s, "ALT-%c", key - (ALFC_KEY_ALT));
		return s;
	}

	if (key >= ALFC_KEY_CTRL + 'A' && key <= ALFC_KEY_CTRL + 'Z')
	{
		s = malloc(32);
		sprintf(s, "CTRL-%c", key - (ALFC_KEY_CTRL));
		return s;
	}

	for (j = 0; keys[j].key != -1; j++)
	{
		if (keys[j].key == key)
		{
			s = malloc(32);
			sprintf(s, "%s", keys[j].name);

			return s;
		}
	}

	if (key >= 0x20 && key <= 0x7F)
	{
		s = malloc(8);
		sprintf(s, "%c", key);
		return s;
	}

	return strdup("(?)");
}

void DrawMenuLine(uScreenDriver *screen, DList *lstHotKeys)
{
	char *buff;
	int m;
	char *q;

	DLElement *e;

	m = screen->get_screen_width();
	buff = malloc(m + 4);
	memset(buff, ' ', m);

	e = dlist_head(lstHotKeys);
	q = buff;

	*q++ = ' ';

	while (e != NULL)
	{
		uKeyBinding *kb;
		char *keyname;

		if (e != dlist_head(lstHotKeys))
			q += 1;

		kb = dlist_data(e);

		keyname = ConvertKeyToName(kb->key);
		if ((q - buff) + strlen(keyname) + strlen(kb->sTitle) + 3 < screen->get_screen_width())
		{
			sprintf(q, "%s - %s", keyname, kb->sTitle);
			q = strchr(buff, 0x0);
			*q++ = ' ';

			if (q - buff < screen->get_screen_width())
				*q++ = '|';
		}

		free(keyname);

		e = dlist_next(e);
	}

	buff[m] = 0;
	screen->set_style(STYLE_TITLE);
	screen->set_cursor(screen->get_screen_height(), 1);
	screen->print(buff);

	free(buff);
}

void DrawStatusInfoLine(uGlobalData *gd)
{
	char *buff;
	int m = gd->screen->get_screen_width();
	char *p;
	char ssize[64];
	char ssize2[64];
	int maxwidth;
	char *x1, *x2;

	assert(gd!=NULL);
	maxwidth = m + 4;
	if (maxwidth < 100)
		maxwidth = 100;

	buff = malloc(maxwidth);

	memset(buff, ' ', m);

	compress_size(ssize, GetActWindow(gd)->tagged_size);
	while (ssize[0] == ' ')
		memmove(ssize, ssize + 1, strlen(ssize) + 1);

	compress_size(ssize2, GetActWindow(gd)->total_size);
	while (ssize2[0] == ' ')
		memmove(ssize2, ssize2 + 1, strlen(ssize2) + 1);

	x1 = PrintNumber(dlist_size(GetActFullList(gd)));
	x2 = PrintNumber(GetActWindow(gd)->hidden_count);

	sprintf(buff, "Tagged : %i file%c (%s) : Size %s, %s Files, %s Hidden", GetActWindow(gd)->tagged_count, GetActWindow(gd)->tagged_count == 1 ? 0 : 's', ssize, ssize2, x1, x2);
	p = strchr(buff, 0x0);
	*p = ' ';
	buff[m] = 0;

	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height() - 2, 1);
	gd->screen->print(buff);

	free(x1);
	free(x2);

	free(buff);
}

void SwitchPanes(uGlobalData *gd)
{
	uDirEntry *de;
	int j;
	uWindow *w;
	int size_off;
	int date_off;
	int max_namelen;

	w = GetActWindow(gd);
	size_off = CalcSizeOff(w, w->width - 3);
	date_off = CalcDateOff(w, size_off);
	max_namelen = date_off - 2;
	de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
	// no files in directory....
	if (de != NULL)
	{
		j = w->highlight_line;
		w->highlight_line = -1;
		PrintFileLine(de, j, w, max_namelen, size_off, date_off);
		w->highlight_line = j;
	}
	else
		DrawNoFile(w, STYLE_NORMAL);

	if (gd->selected_window == WINDOW_RIGHT)
	{
		gd->selected_window = WINDOW_LEFT;
	}
	else
	{
		gd->selected_window = WINDOW_RIGHT;
	}

	w = GetActWindow(gd);
	size_off = CalcSizeOff(w, w->width - 3);
	date_off = CalcDateOff(w, size_off);
	max_namelen = date_off - 2;
	de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
	if (de != NULL)
		PrintFileLine(de, w->highlight_line, w, max_namelen, size_off, date_off);
	else
		DrawNoFile(w, STYLE_HIGHLIGHT);

	chdir(GetActDPath(gd));

	DrawActive(gd);
	DrawFilter(gd);
	DrawStatusInfoLine(gd);
}

void SetActivePane(uGlobalData *gd, int p)
{
	gd->selected_window = p;
	DrawActive(gd);
	DrawFilter(gd);
}

static void exec_internal_command(uGlobalData *gd, char *command)
{
	if (command == NULL)
		return;

	CallGlobalFunc(gd->_GL, "CLIParse", "s", command);
}

void DrawCLI(uGlobalData *gd)
{
	gd->screen->set_style(STYLE_TITLE);
	gd->screen->set_cursor(gd->screen->get_screen_height() - 1, 1);
	gd->screen->print(" > ");

	gd->screen->set_style(STYLE_NORMAL);
	gd->screen->erase_eol();
	gd->screen->set_cursor(gd->screen->get_screen_height() - 1, 4);

	gd->screen->print(gd->command);
	gd->screen->print("~");
}

void DrawFilter(uGlobalData *gd)
{
	DLElement *e;
	char *p;
	char *x;

	x = malloc(16 + gd->screen->get_screen_width());

	memset(x, ' ', gd->screen->get_screen_width());
	memmove(x, "Filter: ", 8);

	if (GetActFilter(gd) == NULL)
		e = NULL;
	else
		e = dlist_head(GetActFilter(gd));

	p = x + 8;

	while (e != NULL)
	{
		char *q;

		q = dlist_data(e);
		if (p > (x + 8))
		{
			*p++ = ',';
			*p++ = ' ';
		}

		memmove(p, q, strlen(q));
		p += strlen(q);

		e = dlist_next(e);
	}

	e = dlist_head(GetActGlob(gd));
	while (e != NULL)
	{
		char *q;

		q = dlist_data(e);
		if (p > (x + 8))
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
	gd->screen->set_cursor(gd->screen->get_screen_height() - 3, 1);
	gd->screen->print(x);
	gd->screen->set_style(STYLE_NORMAL);

	free(x);
}

static int get_scroll_depth(uWindow *w)
{
	if (dlist_size(GetActList(w->gd)) < w->height - 2)
		return dlist_size(GetActList(w->gd));
	else
		return(w->height - 2) -	4;
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

	if (tl < 0)
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
	int size_off;
	int max_namelen;
	int date_off;

	w = GetActWindow(gd);

	if (w->highlight_line == 0 && w->top_line == 0)
	{
		return -1;
	}

	scroll_depth = get_scroll_depth(w);
	size_off = CalcSizeOff(w, w->width - 3);
	date_off = CalcDateOff(w, size_off);
	max_namelen = date_off - 2;

	if ((w->top_line == 0 && w->highlight_line > 0) || (w->highlight_line > 4 && w->top_line > 0))
	{
		uDirEntry *de;

		w->highlight_line -= 1;

		de = GetHighlightedFile(GetActList(gd), w->highlight_line + 1, w->top_line);
		PrintFileLine(de, w->highlight_line + 1, w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
		PrintFileLine(de, w->highlight_line, w, max_namelen, size_off, date_off);

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
	int size_off;
	int date_off;
	int max_namelen;

	w = GetActWindow(gd);

	if (w->top_line + w->highlight_line + 1 >= dlist_size(GetActList(gd)))
		return -1;

	scroll_depth = get_scroll_depth(w);
	size_off = CalcSizeOff(w, w->width - 3);
	date_off = CalcDateOff(w, size_off);
	max_namelen = date_off - 2;


	// hl is 0 based
	if ((w->highlight_line + 1 < scroll_depth) || (w->top_line + 1 + w->highlight_line >= dlist_size(GetActList(gd)) - 4))
	{
		uDirEntry *de;

		w->highlight_line += 1;

		de = GetHighlightedFile(GetActList(gd), w->highlight_line - 1, w->top_line);
		PrintFileLine(de, w->highlight_line - 1, w, max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), w->highlight_line, w->top_line);
		PrintFileLine(de, w->highlight_line, w, max_namelen, size_off, date_off);

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
	char *skip[] =
	{
		"HistoryUp()", "HistoryDown()", ":q", NULL
	};
	int i;

	x = malloc(8192);

	va_list args;
	va_start(args, str);
	vsprintf(x, str, args);
	va_end(args);

	x = realloc(x, strlen(x) + 1);

	for (i = 0; skip[i] != NULL; i++)
	{
		int j = strlen(skip[i]);
		if (strncmp(skip[i], x, j) == 0 && (x[j] == 0 || x[j] == ';'))
		{
			free(x);
			return;
		}
	}

	dlist_ins(gd->lstLogHistory, x);
	gd->hist_idx = dlist_size(gd->lstLogHistory);

	x = INI_get(gd->optfile, "options", "history_count");
	if (x == NULL)
		i = 16;
	else
		i = atoi(x);

	if (i < 0)
		i = 32;

	if (dlist_size(gd->lstLogHistory) > i)
	{
		dlist_remove(gd->lstLogHistory, dlist_tail(gd->lstLogHistory), (void*) &x);
		free(x);
	}
}

int GetFileIndex(DList *lstFiles, char *name)
{
	DLElement *d;
	uDirEntry *de;
	int count;

	if (name == NULL)
		return -1;

	d = dlist_head(lstFiles);
	count = 0;
	while (d != NULL)
	{
		de = dlist_data(d);

		if (strcmp(de->name, name) == 0)
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
	while (d != NULL)
	{
		de = dlist_data(d);

		if (strcmp(de->name, name) == 0)
			return de;

		d = dlist_next(d);
	}

	return NULL;
}

int SetHighlightedFile(uGlobalData *gd, int idx)
{
	int depth = GetActWindow(gd)->height - 2;

	if (idx != -1)
	{
		int new_top = 0;
		int new_hl = idx;
		int size;

		size = dlist_size(GetActList(gd));

		if (size >= depth)
		{
			if (idx > depth / 2 && depth)
			{
				if (idx + (depth / 2) > size)
				{
					new_top = size - depth;
					new_hl = idx - new_top;
				}
				else
				{
					new_top = idx - depth / 2;
					new_hl = idx - new_top;	//(size - (depth / 2)) - (depth % 2);
				}
			}
			else
			{
				new_top = 0;
				new_hl = idx;
			}
		}

		assert(new_top >=0);
		assert(new_hl >= 0);

		GetActWindow(gd)->highlight_line = new_hl;
		GetActWindow(gd)->top_line = new_top;
	}

	return idx;
}

DList* ResetFilteredFileList(uGlobalData *gd, DList *lstF, DList *lstA)
{
	DList *lstB;
	DLElement *e;
	int hidden;

	hidden = IsTrue(INI_get(gd->optfile, "options", "show_hidden"));

	lstB = malloc(sizeof(DList));
	dlist_init(lstB, NULL);

	e = dlist_head(lstA);
	while (e != NULL)
	{
		uDirEntry *de;

		de = dlist_data(e);
		if (hidden == 0 || ALFC_IsHidden(de->name, de->attrs) != 0)
			dlist_ins(lstB, dlist_data(e));

		e = dlist_next(e);
	}

	e = dlist_head(lstF);
	while (e != NULL)
	{
		char *x;

		dlist_remove(lstF, e, (void*) &x);
		free(x);

		e = dlist_head(lstF);
	}

	return lstB;
}

void UpdateDir(uGlobalData *gd, char *set_to_highlight)
{
	gd->screen->set_updates(0);

	if (gd->selected_window == WINDOW_LEFT)
	{
		gd->win_left->total_size = 0;
		gd->win_left->hidden_count = 0;
		gd->win_left->tagged_count = 0;
		gd->win_left->tagged_size = 0;

		free(gd->left_dir);
		gd->left_dir = GetCurrentWorkingDirectory();
		AddMRU(gd, gd->lstMRULeft, gd->left_dir);
		if (gd->lstFullLeft != NULL)
		{
			dlist_destroy(gd->lstFullLeft);
			free(gd->lstFullLeft);
		}
		gd->lstFullLeft = GetFiles(gd->left_dir);
		CalcDirStats(gd, gd->win_left, gd->lstFullLeft);

		if (gd->lstLeft != NULL)
		{
			dlist_destroy(gd->lstLeft);
			free(gd->lstLeft);
		}
		gd->lstLeft = ResetFilteredFileList(gd, gd->lstFilterLeft, gd->lstFullLeft);

		assert(gd->lstFilterLeft != NULL);
		assert(gd->lstFullLeft != NULL);
		assert(gd->lstLeft != NULL);
		assert(gd->lstGlobLeft != NULL);
		UpdateFilterList(gd, gd->lstFilterLeft, gd->lstGlobLeft, gd->lstFullLeft, gd->lstLeft);
		assert(gd->lstLeft != NULL);
	}
	else
	{
		gd->win_right->tagged_count = 0;
		gd->win_right->tagged_size = 0;
		gd->win_right->total_size = 0;
		gd->win_right->hidden_count = 0;

		free(gd->right_dir);
		gd->right_dir = GetCurrentWorkingDirectory();

		AddMRU(gd, gd->lstMRURight, gd->right_dir);
		if (gd->lstFullRight != NULL)
		{
			dlist_destroy(gd->lstFullRight);
			free(gd->lstFullRight);
		}
		gd->lstFullRight = GetFiles(gd->right_dir);
		CalcDirStats(gd, gd->win_right, gd->lstFullRight);

		if (gd->lstRight != NULL)
		{
			dlist_destroy(gd->lstRight);
			free(gd->lstRight);
		}
		gd->lstRight = ResetFilteredFileList(gd, gd->lstFilterRight, gd->lstFullRight);

		assert(gd->lstFilterRight != NULL);
		assert(gd->lstFullRight != NULL);
		assert(gd->lstRight != NULL);
		assert(gd->lstGlobRight != NULL);
		UpdateFilterList(gd, gd->lstFilterRight, gd->lstGlobRight, gd->lstFullRight, gd->lstRight);
	}

	GetActWindow(gd)->top_line = 0;
	GetActWindow(gd)->highlight_line = 0;
	assert(gd->lstLeft != NULL);

	if (set_to_highlight != NULL)
	{
		int idx = GetFileIndex(GetActList(gd), set_to_highlight);
		if (idx >= 0)
			SetHighlightedFile(gd, idx);
	}

	assert(gd->lstLeft != NULL);
	assert(GetActList(gd) != NULL);
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawFilter(gd);
	DrawStatusInfoLine(gd);

	gd->screen->set_updates(1);
}

int change_dir(uGlobalData *gd, char *dir)
{
	char *cpath;


	//cpath = ConvertDirectoryName( dir );
	cpath = replace(dir, '\\', '/');

	if (chdir(cpath) != 0)
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

	cpath = replace(GetActDPath(gd), '\\', '/');

	if (chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	scan = strchr(cpath, 0);
	while (scan > cpath && *scan != '/')
		scan -= 1;

	if (*scan == '/')
		scan += 1;

	if (chdir("..") != 0)
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
	char *cpath1;
	char *cpath2;

	cpath1 = strdup(GetActDPath(gd));
	if (chdir(cpath1) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath1);
		free(cpath1);
		return -1;
	}

	cpath2 = ConvertDirectoryName(dir);
	if (strcmp(cpath1, cpath2) != 0)
	{
		if (chdir(cpath2) != 0)
		{
			free(cpath1);
			free(cpath2);
			LogInfo("Could not change to directory %s\n", cpath2);
			return -1;
		}
	}

	UpdateDir(gd, NULL);
	free(cpath1);
	free(cpath2);

	return 0;
}

int downdir(uGlobalData *gd)
{
	char *cpath;
	uDirEntry *de;

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);

	if (ALFC_IsDir(de->attrs) != 0)
		return -1;

	cpath = strdup(GetActDPath(gd));

	if (chdir(cpath) != 0)
	{
		LogInfo("Could not change to directory %s\n", cpath);
		free(cpath);
		return -1;
	}

	free(cpath);

	if (chdir(de->name) != 0)
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
	if (de->tagged == 1)
	{
		GetActWindow(gd)->tagged_count += 1;
		GetActWindow(gd)->tagged_size += de->size;
	}
	else
	{
		GetActWindow(gd)->tagged_count -= 1;
		GetActWindow(gd)->tagged_size -= de->size;
	}

	gd->screen->set_style(STYLE_HIGHLIGHT);
	gd->screen->set_cursor(GetActWindow(gd)->offset_row + GetActWindow(gd)->highlight_line + 2, GetActWindow(gd)->offset_col + 2);
	gd->screen->print(de->tagged == 1 ? "+" : " ");
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
	int size_off;
	int date_off;
	int max_namelen;

	gd->screen->set_updates(0);

	size_off = CalcSizeOff(GetActWindow(gd), GetActWindow(gd)->width - 3);
	date_off = CalcDateOff(GetActWindow(gd), size_off);
	max_namelen = date_off - 2;

	tl = GetActWindow(gd)->top_line;
	depth = GetActWindow(gd)->height - 2;
	fc = dlist_size(GetActList(gd));
	hl = GetActWindow(gd)->highlight_line;

	if (dlist_size(GetActList(gd)) < depth)
	{
		// file list is smaller than our window size, so just bottom out.
		depth = dlist_size(GetActList(gd));

		GetActWindow(gd)->highlight_line = depth - 1;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, size_off, date_off);

		DrawActiveFileInfo(gd);
	}
	else if (hl < depth - 5)
	{
		// move cursor to bottom of current page
		GetActWindow(gd)->highlight_line = depth - 5;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, size_off, date_off);

		DrawActiveFileInfo(gd);
	}
	else
	{
		// scroll actual page
		int c;

		c = fc - tl;
		c -= hl;

		if (c > depth)
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

	gd->screen->set_updates(1);
}

void scroll_page_up(uGlobalData *gd)
{
	int tl;
	int depth;
	int fc;
	int hl;

	uDirEntry *de;
	int size_off;
	int date_off;
	int max_namelen;

	gd->screen->set_updates(0);

	size_off = CalcSizeOff(GetActWindow(gd), GetActWindow(gd)->width - 3);
	date_off = CalcDateOff(GetActWindow(gd), size_off);
	max_namelen = date_off - 2;

	tl = GetActWindow(gd)->top_line;
	depth = GetActWindow(gd)->height - 2;
	fc = dlist_size(GetActList(gd));
	hl = GetActWindow(gd)->highlight_line;

	if (dlist_size(GetActList(gd)) < depth)
	{
		// file list is smaller than our window size, so just top out.
		depth = dlist_size(GetActList(gd));

		if (GetActWindow(gd)->highlight_line == 0)
			return;

		GetActWindow(gd)->highlight_line = 0;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, size_off, date_off);

		DrawActiveFileInfo(gd);
	}
	else if (hl > 5)
	{
		// move cursor to top of current page
		GetActWindow(gd)->highlight_line = 5;

		de = GetHighlightedFile(GetActList(gd), hl, GetActWindow(gd)->top_line);
		PrintFileLine(de, hl, GetActWindow(gd), max_namelen, size_off, date_off);

		de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
		PrintFileLine(de, GetActWindow(gd)->highlight_line, GetActWindow(gd), max_namelen, size_off, date_off);

		DrawActiveFileInfo(gd);
	}
	else
	{
		// scroll actual page
		if (tl > depth)
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

	gd->screen->set_updates(1);
}

void UpdateFilterList(uGlobalData *gd, DList *lstFilter, DList *lstGlob, DList *lstFull, DList *lstF)
{
	DLElement *e, *e2;
	regex_t preg;
	uDirEntry *de;
	int status;
	DList *lstNew;
	DList *lstOld;

	uDirEntry *deold;

	deold = GetHighlightedFile(lstF, GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);

	if (dlist_size(lstFilter) > 0)
	{
		dlist_empty(lstF);

		lstNew = lstFull;
		lstOld = malloc(sizeof(DList));
		dlist_init(lstOld, NULL);

		e = dlist_head(lstFilter);
		while (e != NULL)
		{
			char *pattern;
			int rc;

			pattern = dlist_data(e);
			rc = regcomp(&preg, pattern, REG_EXTENDED | REG_NOSUB);
			if (rc == 0)
			{
				e2 = dlist_head(lstNew);

				while (e2 != NULL)
				{
					de = dlist_data(e2);

					status = regexec(&preg, de->name, 0, NULL, 0);
					if (status == 0)
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

			if (lstNew != lstFull)
			{
				dlist_destroy(lstNew);
				free(lstNew);
			}

			lstNew = lstOld;

			lstOld = malloc(sizeof(DList));
			dlist_init(lstOld, NULL);

			e = dlist_next(e);
		}

		if (lstOld != lstFull)
		{
			dlist_destroy(lstOld);
			free(lstOld);
		}

		if (lstNew != lstFull)
		{
			dlist_destroy(lstNew);
			free(lstNew);
		}
	}
	else
	{
		if (dlist_size(lstGlob) > 0)
		{
			dlist_empty(lstF);
		}
	}

	UpdateGlobList(gd, lstGlob, lstFull, lstF);

	if (dlist_size(lstF) > 0)
	{

		assert(lstF != NULL);
		SortList(gd, lstF);
		assert(lstF != NULL);

		if (deold != NULL)
			status = GetFileIndex(lstF, deold->name);
		else
			status = GetFileIndex(lstF, NULL);

		if (SetHighlightedFile(gd, status) == -1)
		{
			GetActWindow(gd)->top_line = 0;
			GetActWindow(gd)->highlight_line = 0;
		}
	}

	assert(lstF != NULL);
}

int TagWithGlob(uGlobalData *gd, char *pattern)
{
	DLElement *e;
	uDirEntry *de;
	int count;

	count = 0;
	e = dlist_head(GetActList(gd));
	while (e != NULL)
	{
		de = dlist_data(e);

		if (fnmatch(pattern, de->name, 0) == 0)
		{
			if (de->tagged == 0)
			{
				de->tagged = 1;
				GetActWindow(gd)->tagged_count += 1;
				GetActWindow(gd)->tagged_size += de->size;

				count += 1;
			}
		}

		e = dlist_next(e);
	}

	return count;
}

int TagWithFilter(uGlobalData *gd, char *pattern)
{
	DLElement *e;
	uDirEntry *de;
	int count;
	regex_t preg;
	int rc;

	count = 0;

	if ((rc = regcomp(&preg, pattern, REG_EXTENDED | REG_NOSUB)) == 0)
	{
		e = dlist_head(GetActList(gd));
		while (e != NULL)
		{
			de = dlist_data(e);

			if (regexec(&preg, de->name, 0, NULL, 0) == 0)
			{
				if (de->tagged == 0)
				{
					de->tagged = 1;
					GetActWindow(gd)->tagged_count += 1;
					GetActWindow(gd)->tagged_size += de->size;

					count += 1;
				}
			}

			e = dlist_next(e);
		}

		regfree(&preg);
	}
	else
	{
		char buff[128];
		regerror(rc, &preg, buff, 128);
		LogInfo("regex pattern failed to compile\nError : %s\n", buff);
	}

	return count;
}

static void UpdateGlobList(uGlobalData *gd, DList *lstGlob, DList *lstFull, DList *lstF)
{
	DLElement *e, *e2;
	uDirEntry *de;
	int status;

	DList *lstNew;
	DList *lstOld;

	if (dlist_size(lstGlob) == 0)
		return;

	lstNew = lstFull;
	lstOld = malloc(sizeof(DList));
	dlist_init(lstOld, NULL);

	e = dlist_head(lstGlob);
	while (e != NULL)
	{
		char *pattern;

		pattern = dlist_data(e);

		e2 = dlist_head(lstNew);

		while (e2 != NULL)
		{
			de = dlist_data(e2);

			if (ALFC_IsDir(de->attrs) != 0)
			{
				status = fnmatch(pattern, de->name, 0);
				if (status == 0)
					dlist_ins(lstF, de);
				else
					dlist_ins(lstOld, de);
			}
			else
				dlist_ins(lstF, de);

			e2 = dlist_next(e2);
		}

		if (lstNew != lstFull)
		{
			dlist_destroy(lstNew);
			free(lstNew);
		}

		lstNew = lstOld;

		lstOld = malloc(sizeof(DList));
		dlist_init(lstOld, NULL);

		e = dlist_next(e);
	}

	if (lstOld != lstFull)
	{
		dlist_destroy(lstOld);
		free(lstOld);
	}

	if (lstNew != lstFull)
	{
		dlist_destroy(lstNew);
		free(lstNew);
	}
}

uKeyBinding* ScanKey(DList *lst, int key)
{
	DLElement *e;
	uKeyBinding *kb;

	e = dlist_head(lst);
	while (e != NULL)
	{
		kb = dlist_data(e);

		if (kb->key == key)
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

	assert(gd != NULL);

	gd->screen->set_updates(0);

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawFileListWindow(GetInActWindow(gd), GetInActList(gd), GetInActDPath(gd));

	gd->screen->set_style(STYLE_TITLE);
	for (j = GetActWindow(gd)->height + 2; j < gd->screen->get_screen_height(); j++)
	{
		gd->screen->set_cursor(j, 1);
		gd->screen->erase_eol();
	}
	gd->screen->set_style(STYLE_NORMAL);

	DrawMenuLine(gd->screen, gd->lstHotKeys);
	DrawCLI(gd);

	SwitchPanes(gd);
	SwitchPanes(gd);

	gd->screen->set_updates(1);
}

static void StartDirectoryMode(uGlobalData *gdata, char *start_left, char *start_right)
{
	char *x;
	char *orig_dir;

	orig_dir = GetCurrentWorkingDirectory();

	gdata->selected_window = WINDOW_LEFT;
	gdata->lstFilterLeft = malloc(sizeof(DList));
	dlist_init(gdata->lstFilterLeft, FreeFilter);
	gdata->lstGlobLeft = malloc(sizeof(DList));
	dlist_init(gdata->lstGlobLeft, FreeGlob);
	free(gdata->left_dir);

	if (start_left != NULL)
		gdata->left_dir = strdup(start_left);
	else
	{
		x = GetOptionDir(gdata, "left_startup", gdata->lstMRULeft);
		gdata->left_dir = ConvertDirectoryName(x);
		free(x);
	}

	if (gdata->left_dir == NULL)
		gdata->left_dir = GetCurrentWorkingDirectory();

	if (godir(gdata, gdata->left_dir) == -1)
		godir(gdata, ConvertDirectoryName("$HOME"));

	if (gdata->lstLeft == NULL)
	{
		gdata->left_dir = GetCurrentWorkingDirectory();
		godir(gdata, gdata->left_dir);
	}
	assert(gdata->lstLeft != NULL);


	/* now do right window stuff */

	chdir(orig_dir);

	gdata->selected_window = WINDOW_RIGHT;
	gdata->lstFilterRight = malloc(sizeof(DList));
	dlist_init(gdata->lstFilterRight, FreeFilter);
	gdata->lstGlobRight = malloc(sizeof(DList));
	dlist_init(gdata->lstGlobRight, FreeGlob);
	free(gdata->right_dir);

	if (start_right != NULL)
		gdata->right_dir = strdup(start_right);
	else
	{
		x = GetOptionDir(gdata, "right_startup", gdata->lstMRURight);
		gdata->right_dir = ConvertDirectoryName(x);
		free(x);
	}

	if (gdata->right_dir == NULL)
		gdata->right_dir = GetCurrentWorkingDirectory();
	if (godir(gdata, gdata->right_dir) == -1)
	{
		godir(gdata, ConvertDirectoryName("$HOME"));
	}
	if (gdata->lstRight == NULL)
	{
		gdata->right_dir = GetCurrentWorkingDirectory();
		godir(gdata, gdata->right_dir);
	}
	assert(gdata->lstRight != NULL);

	gdata->selected_window = WINDOW_LEFT;
	chdir(gdata->left_dir);

	free(orig_dir);
}

int ScanMenuOpen(uGlobalData *gd, uint32_t key)
{
	int i;
	uMenu **menu;

	menu = GetActMenu(gd);

	for (i = 0; i < MAX_MENU; i++)
	{
		if (menu[i] != NULL)
		{
			if (menu[i]->key == key)
				return i;
		}
	}

	return -1;
}

static void run_exec_command(uGlobalData *gd, char *sCmd)
{
	AddHistory(gd, sCmd);

	// exec string via lua...
	if (sCmd[0] == ':')
	{
		exec_internal_command(gd, sCmd);
	}
	else if (sCmd[0] == '@')
	{
		char *fn = ConvertDirectoryName(sCmd + 1);
		ExecuteScript(gd, fn);
		free(fn);
	}
	else
	{
		//ExecuteString(gd, sCmd);
		exec_internal_command(gd, sCmd);
	}
}

int ALFC_main(int start_mode, char *view_file)
{
	uGlobalData *gdata;

	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR | LOG_STDERR, 5000);
	LogInfo("ALFC :%s- Stu George\nVersion v%i.%02i/%04i - Built on " __DATE__ "; " __TIME__ "\n", gstr_WindowTitle, VersionMajor(), VersionMinor(), VersionBuild());

	ALFC_startup();

#ifndef __WIN32__
	setenv("ALFC", "$HOME/.alfc/scripts", 0);
#endif

	gdata = NewGlobalData();

	if (gdata != NULL)
	{
		int rc;

		gdata->mode = start_mode;

		gdata->lstFileOps = malloc(sizeof(DList));
		dlist_init(gdata->lstFileOps, FreeFileOp);

		gdata->lstHotKeys = malloc(sizeof(DList));
		dlist_init(gdata->lstHotKeys, FreeKey);

		SetupStartDirectories(gdata);
		LoadOptions(gdata);

		gdata->screen = malloc(sizeof(uScreenDriver));
		memmove(gdata->screen, &screen, sizeof(uScreenDriver));

		gdata->screen->gd = gdata;
		gdata->screen->init(gdata->screen);
		gdata->screen->set_updates(1);

		gdata->screen->init_style(STYLE_HELP_TITLE, CLR_WHITE, CLR_CYAN);
		gdata->screen->init_style(STYLE_HELP_NORMAL, CLR_BLACK, CLR_CYAN);
		gdata->screen->init_style(STYLE_HELP_BOLD, CLR_BLUE, CLR_CYAN);
		gdata->screen->init_style(STYLE_HELP_EMPHASIS, CLR_WHITE, CLR_CYAN);
		gdata->screen->init_style(STYLE_HELP_LINK, CLR_YELLOW, CLR_CYAN);

		gdata->screen->set_style(STYLE_NORMAL);
		gdata->screen->cls();

		gdata->screen->set_style(STYLE_TITLE);
		gdata->screen->set_cursor(1, ((gdata->screen->get_screen_width() - (strlen(gstr_WindowTitle) - 8)) / 2));
		gdata->screen->print(gstr_WindowTitle);
		BuildWindowLayout(gdata);


		// screen too small to show nds columns
		// date coumns waste too much space.
		// todo - separate the date/time columns??
		//LogInfo("Screen is %i.%i\n", gdata->screen->get_screen_width(), gdata->screen->get_screen_height());

		if (gdata->screen->get_screen_width() <= 80)
		{
			if (strlen(gdata->columns) > 2)
			{
				LogInfo("Screen not wide enough for that many columns\nReducing to just name/size\n");
				strcpy(gdata->columns, "ns");
			}
		}
		else if (gdata->screen->get_screen_width() > 80 && gdata->screen->get_screen_width() < 100)
		{
			if (strlen(gdata->columns) >= 3)
			{
				LogInfo("Reducing date/time format because of screen size\n");
				strcpy(gdata->columns, "nds");

				ParseTimeFormat(gdata, "HH:mm");
				ParseDateFormat(gdata, "yymmdd");
			}
		}

		LogWrite_SetFlags(LogWrite_GetFlags() & ~LOG_STDERR);


		// reset mode so we always load the global lua
		int old_mode = gdata->mode;
		gdata->mode = eMode_Directory;
		// load it, if it fails, try looking in home as a fallback
		rc = LoadGlobalScript(gdata, INI_get(gdata->optfile, "scripts", "global_funcs"));
		if (rc < 0)
		{
			rc = LoadGlobalScript(gdata, "$HOME/.alfc/global.lua");
		}
		gdata->mode = old_mode;

		if (rc == 0)
		{
			LogInfo("\n" LUA_RELEASE "; " LUA_COPYRIGHT "\n" LUA_AUTHORS "\n\n");

			if (gdata->screen->get_screen_width() < 60 || gdata->screen->get_screen_height() < 20)
			{
				int r, c;

				c = gdata->screen->get_screen_width();
				r = gdata->screen->get_screen_height();
				gdata->screen->deinit();
				LogError("Display is %i.%i, it must be at least 20x60", r, c);
				exit(1);
			}

			ALFC_GetUserInfo(gdata);
			ExecStartupScript(gdata);

			if (gdata->mode == eMode_Directory)
			{
				StartDirectoryMode(gdata, start_left, start_right);
				DrawAll(gdata);
			}

			if (gdata->mode == eMode_Viewer && view_file != NULL)
			{
				ViewFile(gdata, view_file, NULL);
				gdata->screen->init_dir_styles(gdata->screen);
				intFlag = 1;
			}

			if(gdata->mode == eMode_VB_List)
			{
				ListFile(gdata);
				gdata->screen->init_dir_styles(gdata->screen);
				intFlag = 1;
			}

			while (intFlag == 0 && gdata->screen->screen_isshutdown() == 0)
			{
				uint32_t key;
				uKeyBinding *kb;
				int mnu;

				key = 0;
				if (gdata->screen->screen_isresized() != 0)
				{
					char *hr, *hl;

					hr = NULL;
					hl = NULL;

					if (GetActList(gdata ) != NULL)
					{
						uDirEntry *deR, *deL;

						deR = NULL;
						deL = NULL;

						deL = GetHighlightedFile(gdata->lstLeft, gdata->win_left->highlight_line, gdata->win_left->top_line);
						deR = GetHighlightedFile(gdata->lstRight, gdata->win_right->highlight_line, gdata->win_right->top_line);

						hl = deL->name;
						hr = deR->name;
					}

					gdata->screen->cls();
					BuildWindowLayoutLeft(gdata->win_left, hl);
					BuildWindowLayoutRight(gdata->win_right, hr);
					gdata->screen->set_style(STYLE_NORMAL);
					gdata->screen->cls();
					DrawAll(gdata);
				}
				else
				{
					gdata->screen->set_cursor(gdata->screen->get_screen_height() - 1, 4 + strlen(gdata->command));
					key = gdata->screen->get_keypress();
				}

				if (key != 0)
				{
					mnu = ScanMenuOpen(gdata, key);
					if (mnu != -1)
					{
						DrawMenu(gdata, NULL, mnu);
						DrawAll(gdata);
					}
					else
					{
						kb = ScanKey(gdata->lstHotKeys, key);
						if (kb != NULL)
						{
							run_exec_command(gdata, kb->sCommand);
						}
						else
						{
							switch (key)
							{

								case ALFC_KEY_ESCAPE_ESCAPE:	// ESC-ESC
									intFlag = 1;
									break;

								case ALFC_KEY_TAB: // TAB
									SwitchPanes(gdata);
									break;

								case ALFC_KEY_ENTER:
									//if (gdata->command_length > 0)
									{
										run_exec_command(gdata, gdata->command);

										gdata->command_length = 0;
										gdata->command[gdata->command_length] = 0;
										DrawAll(gdata);
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
									if (key == 0)
										break;


									// terminal could send ^H (0x08) or ASCII DEL (0x7F)
									if ((key == ALFC_KEY_BACKSPACE || key == ALFC_KEY_DEL) || (key >= ' ' && key <= 0x7F))
									{
										if ((key == ALFC_KEY_DEL || key == ALFC_KEY_BACKSPACE))
										{
											if (gdata->command_length > 0)
											{
												gdata->command_length -= 1;
												gdata->command[gdata->command_length] = 0;
											}
										}
										else
										{
											if (gdata->command_length < gdata->screen->get_screen_width() - 10)
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
				}
			}

			gdata->screen->set_style(STYLE_NORMAL);
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

		FreeMenus(gdata);

		if (gdata->lstFileOps != NULL)
		{
			dlist_destroy(gdata->lstFileOps);
			free(gdata->lstFileOps);
		}

		if (gdata->lstHotKeys != NULL)
		{
			dlist_destroy(gdata->lstHotKeys);
			free(gdata->lstHotKeys);
		}

		if (gdata->_GL != NULL)
			lua_close(gdata->_GL);

		if (gdata->gcode != NULL)
			free(gdata->gcode);

		if (gdata->optfile != NULL)
			INI_unload(gdata->optfile);

		if (gdata->screen != NULL)
		{
			gdata->screen->disable_raw();
			gdata->screen->deinit();
			free(gdata->screen);
		}

		if (gdata->startup_path != NULL)
		{
			chdir(gdata->startup_path);
			free(gdata->startup_path);
		}

		if (gdata->lstMRULeft != NULL)
		{
			dlist_destroy(gdata->lstMRULeft);
			free(gdata->lstMRULeft);
		}

		if (gdata->lstMRURight != NULL)
		{
			dlist_destroy(gdata->lstMRURight);
			free(gdata->lstMRURight);
		}

		if (gdata->lstLogHistory != NULL)
		{
			dlist_destroy(gdata->lstLogHistory);
			free(gdata->lstLogHistory);
		}

		if (gdata->lstLeft != NULL)
		{
			dlist_destroy(gdata->lstLeft);
			free(gdata->lstLeft);
		}

		if (gdata->lstRight != NULL)
		{
			dlist_destroy(gdata->lstRight);
			free(gdata->lstRight);
		}

		if (gdata->lstFullLeft != NULL)
		{
			dlist_destroy(gdata->lstFullLeft);
			free(gdata->lstFullLeft);
		}

		if (gdata->lstFullRight != NULL)
		{
			dlist_destroy(gdata->lstFullRight);
			free(gdata->lstFullRight);
		}

		if (gdata->lstFilterLeft != NULL)
		{
			dlist_destroy(gdata->lstFilterLeft);
			free(gdata->lstFilterLeft);
		}

		if (gdata->lstFilterRight != NULL)
		{
			dlist_destroy(gdata->lstFilterRight);
			free(gdata->lstFilterRight);
		}

		if (gdata->lstGlobLeft != NULL)
		{
			dlist_destroy(gdata->lstGlobLeft);
			free(gdata->lstGlobLeft);
		}

		if (gdata->lstGlobRight != NULL)
		{
			dlist_destroy(gdata->lstGlobRight);
			free(gdata->lstGlobRight);
		}

		if (gdata->win_left != NULL)
			free(gdata->win_left);
		if (gdata->win_right != NULL)
			free(gdata->win_right);

		if (gdata->left_dir != NULL)
			free(gdata->left_dir);

		if (gdata->right_dir != NULL)
			free(gdata->right_dir);

		if (gdata->optfilename != NULL)
			free(gdata->optfilename);

		if (gdata->optfilehistory != NULL)
			free(gdata->optfilehistory);

		if (gdata->strRealName != NULL)
			free(gdata->strRealName);
		if (gdata->strLoginName != NULL)
			free(gdata->strLoginName);
		if (gdata->strHomeDirectory != NULL)
			free(gdata->strHomeDirectory);
		if (gdata->strShell != NULL)
			free(gdata->strShell);

		free(gdata);
	}

	ALFC_shutdown();
	LogWrite_Shutdown();

	return 0;
}

