#include <sys/stat.h>
#include <sys/types.h>

#include "headers.h"
#include "defaults.h"

int ParseTimeFormat(uGlobalData *gd, char *tf)
{
	struct udtTimeF
	{
		char *f;
		char sigs[16];
	} tfx[]=
	{
		{ "HH:mm.ss", 			{ 8, et_Hour24, ':', et_Min, '.', et_Sec, 0 }},
		{ "hh:mm.ss AMPM", 		{ 11, et_Hour12, ':', et_Min, '.', et_Sec, ' ', et_AMPM, 0 }},
		{ "hh:mm.ssAMPM", 		{ 11, et_Hour12, ':', et_Min, '.', et_Sec, et_AMPM, 0 }},
		{ "hh:mm.ss", 			{ 8, et_Hour12, ':', et_Min, '.', et_Sec, 0 }},
		{ "HH:mm",	 			{ 5, et_Hour24, ':', et_Min, 0 }},
		{ "hh:mm",	 			{ 5, et_Hour12, ':', et_Min, 0 }},
		{ "hh:mm AMPM",	 		{ 8, et_Hour12, ':', et_Min, ' ', et_AMPM, 0 }},
		{ "hh:mmAMPM",	 		{ 8, et_Hour12, ':', et_Min, et_AMPM, 0 }},
		{ NULL, {0}}
	};

	int i;

	for(i=0; tfx[i].f != NULL; i++)
	{
		if( strcmp(tfx[i].f, tf) == 0)
		{
			memmove(gd->time_fmt, tfx[i].sigs + 1, 16);
			gd->time_fmt_len = tfx[i].sigs[0];
			return i;
		}
	}

	return -1;
}

int ParseDateFormat(uGlobalData *gd, char *tf)
{
	struct udtDateF
	{
		char *f;
		char sigs[16];
	} tfx[]=
	{
		{ "dd/mm/yyyy", 	{ 10, et_Day, '/', et_Month, '/', et_Year4, 0 }},
		{ "dd/mm/yy", 		{ 8, et_Day, '/', et_Month, '/', et_Year2, 0 }},
		{ "mm/dd/yyyy", 	{ 10, et_Month, '/', et_Day, '/', et_Year4, 0 }},
		{ "mm/dd/yy", 		{ 8, et_Month, '/', et_Day, '/', et_Year2, 0 }},
		{ "mm-dd-yyyy", 	{ 10, et_Month, '-', et_Day, '-', et_Year4, 0 }},
		{ "mm-dd-yy", 		{ 8, et_Month, '-', et_Day, '-', et_Year2, 0 }},
		{ "dd-mm-yyyy", 	{ 10, et_Day, '-', et_Month, '-', et_Year4, 0 }},
		{ "dd-mm-yy", 		{ 8, et_Day, '-', et_Month, '-', et_Year2, 0 }},

		{ "dd/mmm/yyyy", 	{ 11, et_Day, '/', et_MonthNameShort, '/', et_Year4, 0 }},
		{ "dd/mmm/yy", 		{ 9, et_Day, '/', et_MonthNameShort, '/', et_Year2, 0 }},
		{ "mmm/dd/yyyy", 	{ 11, et_MonthNameShort, '/', et_Day, '/', et_Year4, 0 }},
		{ "mmm/dd/yy", 		{ 9, et_MonthNameShort, '/', et_Day, '/', et_Year2, 0 }},
		{ "mmm-dd-yyyy", 	{ 11, et_MonthNameShort, '-', et_Day, '-', et_Year4, 0 }},
		{ "mmm-dd-yy", 		{ 9, et_MonthNameShort, '-', et_Day, '-', et_Year2, 0 }},
		{ "dd-mmm-yyyy", 	{ 11, et_Day, '-', et_MonthNameShort, '-', et_Year4, 0 }},
		{ "dd-mmm-yy", 		{ 9, et_Day, '-', et_MonthNameShort, '-', et_Year2, 0 }},

		{ "dd/mmmm/yyyy", 	{ 16, et_Day, '/', et_MonthNameFull, '/', et_Year4, 0 }},
		{ "dd/mmmm/yy", 	{ 14, et_Day, '/', et_MonthNameFull, '/', et_Year2, 0 }},
		{ "mmmm/dd/yyyy", 	{ 16, et_MonthNameFull, '/', et_Day, '/', et_Year4, 0 }},
		{ "mmmm/dd/yy", 	{ 14, et_MonthNameFull, '/', et_Day, '/', et_Year2, 0 }},
		{ "mmmm-dd-yyyy", 	{ 16, et_MonthNameFull, '-', et_Day, '-', et_Year4, 0 }},
		{ "mmmm-dd-yy", 	{ 14, et_MonthNameFull, '-', et_Day, '-', et_Year2, 0 }},
		{ "dd-mmmm-yyyy", 	{ 16, et_Day, '-', et_MonthNameFull, '-', et_Year4, 0 }},
		{ "dd-mmmm-yy", 	{ 14, et_Day, '-', et_MonthNameFull, '-', et_Year2, 0 }},


		{ "yymmdd", 		{ 6, et_Year2, et_Month, et_Day, 0 }},
		{ "yyyymmdd", 		{ 8, et_Year4, et_Month, et_Day, 0 }},
		{ "ddmmyyyy", 		{ 8, et_Day, et_Month, et_Year4, 0 }},
		{ "ddmmmyyyy", 		{ 8, et_Day, et_MonthNameShort, et_Year4, 0 }},

		{ NULL, {0}}
	};

	int i;

	for(i=0; tfx[i].f != NULL; i++)
	{
		if( strcmp(tfx[i].f, tf) == 0)
		{
			memmove(gd->date_fmt, tfx[i].sigs + 1, 16);
			gd->date_fmt_len = tfx[i].sigs[0];
			return i;
		}
	}

	return -1;
}

int IsTrue(const char *s)
{
	char *p;
	char *q;
	char *z;
	int rc;

	if(s == NULL)
		return -1;

	p = strdup(s);
	q = p;

	while(*q != 0 && (*q == 0x20 || *q == 0x09))
		q++;

	z = strchr(p, 0x0);
	z -= 1;

	while(z > p && (*z == 0x20 || *z == 0x09))
	{
		*z = 0;
		z -= 1;
	}


	rc = -1;
	if(stricmp(q, "true") == 0)
		rc = 0;
	else if(stricmp(q, "yes") == 0)
		rc = 0;

	free(p);

	return rc;
}

void CreateHomeDirectory(void)
{
	char *q;

	q = ConvertDirectoryName("$HOME/.alfc");

	ALFC_mkdir(q);

	free(q);
}

static void CreateBaselineINIFile(uGlobalData *gdata)
{
	assert(gdata->optfile != NULL);

	INI_UpdateItem(gdata->optfile, "options", "remember_dirs", "true");
	INI_UpdateItem(gdata->optfile, "options", "dates", "dd/mm/yyyy");
	INI_UpdateItem(gdata->optfile, "options", "times", "hh:mm.ss AMPM");
	INI_UpdateItem(gdata->optfile, "options", "columns", " name,date,size");

	INI_UpdateItem(gdata->optfile, "mru_left", "count", "1");
	INI_UpdateItem(gdata->optfile, "mru_left", "mru0", "$HOME");
	INI_UpdateItem(gdata->optfile, "mru_right", "count", "1");
	INI_UpdateItem(gdata->optfile, "mru_right", "mru0", "$HOME");

	INI_UpdateItem(gdata->optfile, "scripts", "startup_file", "$HOME/.alfc/startup.lua");
	INI_UpdateItem(gdata->optfile, "scripts", "shutdown_file", "$HOME/.alfc/shutdown.lua");

	INI_UpdateItem(gdata->optfile, "colours", "background", "black");
	INI_UpdateItem(gdata->optfile, "colours", "foreground", "grey");

	INI_UpdateItem(gdata->optfile, "colours", "title_fg", "white");
	INI_UpdateItem(gdata->optfile, "colours", "title_bg", "blue");

	INI_UpdateItem(gdata->optfile, "colours", "hi_fg", "yeloow");
	INI_UpdateItem(gdata->optfile, "colours", "hi_bg", "red");

}

int decode_colour(char *s, int def)
{
	int i;

	struct uc{
		char *c;
		int b;
	} cols[] =
	{
		{"black", CLR_BLACK},
		{"blue", CLR_BLUE},
		{"red", CLR_RED},
		{"green", CLR_GREEN},
		{"brown", CLR_BROWN},
		{"grey", CLR_GREY},
		{"cyan", CLR_CYAN},
		{"magenta", CLR_MAGENTA},

		{"darkgrey", CLR_DK_GREY},
		{"brightred", CLR_BR_RED},
		{"green", CLR_BR_GREEN},
		{"yellow", CLR_YELLOW},
		{"brightblue", CLR_BR_BLUE},
		{"brightmagenta", CLR_BR_MAGENTA},
		{"brightcyan", CLR_BR_CYAN},
		{"white", CLR_WHITE},

		{NULL, 0}
	};

	if(s == NULL) return def;

	for(i=0; cols[i].c != NULL; i++)
	{
		if(stricmp(cols[i].c, s) == 0)
		{
			return cols[i].b;
		}
	}

	return def;
}

static void FreeLogEntry(void *x)
{
	free(x);
}

void SaveHistory(uGlobalData *gd)
{
	DLElement *e;
	int i;
	INIFILE *opthist;
	char buff[32];
	char *x;

	opthist = INI_load(gd->optfilehistory);
	if(opthist == NULL)
		opthist = INI_EmptyINF();

	sprintf(buff, "%i", dlist_size(gd->lstLogHistory));
	INI_UpdateItem(opthist, "history", "count", buff);

	e = dlist_head(gd->lstLogHistory);
	i = 0;

	while(e != NULL)
	{
		x = dlist_data(e);
		e = dlist_next(e);

		sprintf(buff, "entry%i", i);
		INI_UpdateItem(opthist, "history", buff, x);

		i += 1;
	}

	INI_save(gd->optfilehistory, opthist);
	INI_unload(opthist);
}

static void LoadHistory(uGlobalData *gd)
{
	int c;
	char *x;
	int i;
	INIFILE *opthist;

	gd->optfilehistory = ConvertDirectoryName("$HOME/.alfc/history.ini");
	opthist = INI_load(gd->optfilehistory);

	gd->lstLogHistory = malloc(sizeof(DList));
	dlist_init(gd->lstLogHistory, FreeLogEntry);

	if(opthist == NULL)
		return;

	x = INI_get(opthist, "history", "count");
	if(x != NULL)
		c = atoi(x);
	else
		c = 0;

	// fake to prime loop to pass test..
	x = (char*)opthist;

	i = 0;
	while(i < c && x != NULL)
	{
		char buff[32];

		sprintf(buff, "entry%i", i);
		x = INI_get(opthist, "history", buff);
		if(x != NULL)
			AddHistory(gd, x);

		i += 1;
	}

	INI_unload(opthist);
}

void SaveMRU(uGlobalData *gd, DList *lst, char *opt)
{
	DLElement *e;
	int i;
	char buff[32];
	char *x;

	sprintf(buff, "%i", dlist_size(lst));
	INI_UpdateItem(gd->optfile, opt, "count", buff);

	e = dlist_head(lst);
	i = 0;

	while(e != NULL)
	{
		x = dlist_data(e);
		e = dlist_next(e);

		sprintf(buff, "mru%i", i);
		INI_UpdateItem(gd->optfile, opt, buff, x);

		i += 1;
	}
}

static void FreeMRU(void *x)
{
	free(x);
}

static void LoadMRU(uGlobalData *gd, DList *lst, char *opt)
{
	int c;
	int i;
	char *x;

	x = INI_get(gd->optfile, opt, "count");
	if(x != NULL)
		c = atoi(x);
	else
		c = 0;


	x = INI_get(gd->optfile, "options", "mru_count");
	if(x != NULL && atoi(x) >= 4 && atoi(x) < c)
		c = atoi(x);

	// fake to pass loop test
	x = (char*)gd;

	i = 0;
	while(i < c && x != NULL)
	{
		char buff[32];

		sprintf(buff, "mru%i", i);
		x = INI_get(gd->optfile, opt, buff);
		if(x != NULL)
			dlist_ins(lst, strdup(x));

		i += 1;
	}
}

static void CreateIfNotExist(char *fn, uint8_t *data, int len)
{
	char *x;
	FILE *fp;

	x = ConvertDirectoryName(fn);
	fp = fopen(x, "r");
	if(fp == NULL)
	{
		fp = fopen(x, "wt");
		if(fp != NULL)
		{
			LogInfo("** Creating : %s\n", x);
			fwrite(data, 1, len, fp);
			fclose(fp);
		}
	}
	else
		fclose(fp);

	free(x);
}

void LoadOptions(uGlobalData *gdata)
{
	char *x;
	char *fmt;


	gdata->optfilename = ConvertDirectoryName("$HOME/.alfc/options.ini");
	gdata->optfile = INI_load(gdata->optfilename);

	if(gdata->optfile == NULL)
	{
		CreateHomeDirectory();
		CreateIfNotExist("$HOME/.alfc/options.ini", include_options_ini, include_options_ini_SIZE);
		CreateIfNotExist("$HOME/.alfc/global.lua", include_global_lua, include_global_lua_SIZE);
		CreateIfNotExist("$HOME/.alfc/viewer.lua", include_viewer_lua, include_viewer_lua_SIZE);
		CreateIfNotExist("$HOME/.alfc/startup.lua", include_startup_lua, include_startup_lua_SIZE);
		CreateIfNotExist("$HOME/.alfc/shutdown.lua", include_shutdown_lua, include_shutdown_lua_SIZE);

		CreateIfNotExist("$HOME/.alfc/core_extract.lua", core_extract_lua, core_extract_lua_SIZE);
		CreateIfNotExist("$HOME/.alfc/core_hash.lua", core_hash_lua, core_hash_lua_SIZE);

		CreateIfNotExist("$HOME/.alfc/viewer_languages.lua", viewer_languages_lua, viewer_languages_lua_SIZE);


		gdata->optfile = INI_load(gdata->optfilename);
	}

	if(gdata->optfile == NULL)
	{
		gdata->optfile = INI_EmptyINF();
		CreateBaselineINIFile(gdata);
	}

	gdata->lstMRULeft = malloc(sizeof(DList));
	gdata->lstMRURight = malloc(sizeof(DList));
	dlist_init(gdata->lstMRULeft, FreeMRU);
	dlist_init(gdata->lstMRURight, FreeMRU);

	x = INI_get(gdata->optfile, "options", "compress_filesize");
	if(x != NULL && IsTrue(x) == 0)
		gdata->compress_filesize = 1;

	x = INI_get(gdata->optfile, "colours", "background");
	gdata->clr_background = decode_colour(x, CLR_GREY);
	x = INI_get(gdata->optfile, "colours", "foreground");
	gdata->clr_foreground = decode_colour(x, CLR_BLACK);

	x = INI_get(gdata->optfile, "colours", "title_fg");
	gdata->clr_title_fg = decode_colour(x, CLR_CYAN);
	x = INI_get(gdata->optfile, "colours", "title_bg");
	gdata->clr_title_bg = decode_colour(x, CLR_BLACK);

	x = INI_get(gdata->optfile, "colours", "hi_fg");
	gdata->clr_hi_foreground = decode_colour(x, CLR_MAGENTA);
	x = INI_get(gdata->optfile, "colours", "hi_bg");
	gdata->clr_hi_background = decode_colour(x, CLR_BLACK);

	LoadMRU(gdata, gdata->lstMRULeft, "mru_left");
	LoadMRU(gdata, gdata->lstMRURight, "mru_right");

	memset(gdata->columns, 0x0, 16);
	x = INI_get(gdata->optfile, "options", "columns");
	if(x != NULL)
	{
		int c = 0;

		while(*x != 0)
		{
			if(strncmp(x, "name", 4) == 0 && (x[4] == 0 || x[4]==' ' || x[4]==','))
			{
				gdata->columns[c++] = 'n';
				x += 4;
				while(*x == ' ' || *x == ',')
					x++;
			}
			else if(strncmp(x, "date", 4) == 0 && (x[4] == 0 || x[4]==' ' || x[4]==','))
			{
				gdata->columns[c++] = 'd';
				x += 4;
				while(*x == ' ' || *x == ',')
					x++;
			}
			else if(strncmp(x, "size", 4) == 0 && (x[4] == 0 || x[4]==' ' || x[4]==','))
			{
				gdata->columns[c++] = 's';
				x += 4;
				while(*x == ' ' || *x == ',')
					x++;
			}
			else
			{
				LogInfo("Unknown column at %s\n", x);
				strcpy(gdata->columns, "nds");
				break;
			}
		}
	}
	else
		strcpy(gdata->columns, "nds");

	x = INI_get(gdata->optfile, "options", "dates");
	if(x != NULL)
		fmt = x;
	else
		fmt = "dd/mm/yyyy";

	if( ParseDateFormat(gdata, fmt) == -1  )
		ParseDateFormat(gdata, "dd/mm/yyyy");


	x = INI_get(gdata->optfile, "options", "times");
	if(x != NULL)
		fmt = x;
	else
		fmt = "hh:mm.ss AMPM";

	if(ParseTimeFormat(gdata, fmt) == -1)
		ParseTimeFormat(gdata, "hh:mm.ss AMPM");

	LoadHistory(gdata);

	if( INI_get(gdata->optfile, "options", "sort_order") == NULL)
		INI_UpdateItem(gdata->optfile, "options", "sort_order", "name_asc");
}

void SaveOptions(uGlobalData *gdata)
{
	//CreateHomeDirectory();
	INI_save(gdata->optfilename, gdata->optfile);
}

void RememberDirectories(uGlobalData *gd)
{
	/*
	char *x;
	x = INI_get(gd->optfile, "options", "remember_dirs");

	if(x != NULL)
	{
		if( IsTrue(x) == 0)
		{
			INI_UpdateItem(gd->optfile, "options", "left_startup", gd->left_dir);
			INI_UpdateItem(gd->optfile, "options", "right_startup", gd->right_dir);
		}
	}
	*/
}
