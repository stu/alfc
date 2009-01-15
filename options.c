#include <sys/stat.h>
#include <sys/types.h>

#ifdef __MINGW_H
// needed for mkdir on mingw
#include <io.h>
#endif

#include "headers.h"
#include "stucore/stucore_config.h"

int IsTrue(const char *s)
{
	char *p;
	char *q;
	char *z;
	int rc;

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

#ifndef __MINGW_H
	mkdir(q, 0744);
#else
	mkdir(q);
#endif
	free(q);
}

static void CreateBaselineINIFile(uGlobalData *gdata)
{
	assert(gdata->optfile != NULL);

	INI_UpdateItem(gdata->optfile, "options", "remember_dirs", "true");

	INI_UpdateItem(gdata->optfile, "columns", "count", "3");
	INI_UpdateItem(gdata->optfile, "columns", "col0", "name");
	INI_UpdateItem(gdata->optfile, "columns", "col1", "date");
	INI_UpdateItem(gdata->optfile, "columns", "col2", "permissions");

	INI_UpdateItem(gdata->optfile, "mru_left", "count", "1");
	INI_UpdateItem(gdata->optfile, "mru_left", "mru0", "$HOME");
	INI_UpdateItem(gdata->optfile, "mru_right", "count", "1");
	INI_UpdateItem(gdata->optfile, "mru_right", "mru0", "$HOME");

	INI_UpdateItem(gdata->optfile, "keydef_modifiers", "modifiers", "4");
	INI_UpdateItem(gdata->optfile, "keydef_modifiers", "modifier0", "ESC");
	INI_UpdateItem(gdata->optfile, "keydef_modifiers", "modifier1", "CTRL");
	INI_UpdateItem(gdata->optfile, "keydef_modifiers", "modifier2", "ALT");
	INI_UpdateItem(gdata->optfile, "keydef_modifiers", "modifier4", "WINDOWS");

	INI_UpdateItem(gdata->optfile, "keydefs", "quit", "q");
	INI_UpdateItem(gdata->optfile, "keydefs", "edit", "e");
	INI_UpdateItem(gdata->optfile, "keydefs", "delete", "d");
	INI_UpdateItem(gdata->optfile, "keydefs", "copy", "c");
	INI_UpdateItem(gdata->optfile, "keydefs", "move", "m");
	INI_UpdateItem(gdata->optfile, "keydefs", "rename", "n");
	INI_UpdateItem(gdata->optfile, "keydefs", "mkdir", "M");
	INI_UpdateItem(gdata->optfile, "keydefs", "rmdir", "R");
	INI_UpdateItem(gdata->optfile, "keydefs", "switch", "TAB");
	INI_UpdateItem(gdata->optfile, "keydefs", "open", "ENTER");
	INI_UpdateItem(gdata->optfile, "keydefs", "toggle_tag", "SPACE");
	INI_UpdateItem(gdata->optfile, "keydefs", "homedir", "H");

	INI_UpdateItem(gdata->optfile, "scripts", "startup_file", "$HOME/.alfc/startup.lua");
	INI_UpdateItem(gdata->optfile, "scripts", "shutdown_file", "$HOME/.alfc/shutdown.lua");

	INI_UpdateItem(gdata->optfile, "colours", "background", "black");
	INI_UpdateItem(gdata->optfile, "colours", "foreground", "grey");

	INI_UpdateItem(gdata->optfile, "colours", "title_fg", "white");
	INI_UpdateItem(gdata->optfile, "colours", "title_bg", "magenta");

	INI_UpdateItem(gdata->optfile, "colours", "hi_fg", "white");
	INI_UpdateItem(gdata->optfile, "colours", "hi_bg", "blue");
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

void LoadOptions(uGlobalData *gdata)
{
	char *x;

	gdata->optfilename = ConvertDirectoryName("$HOME/.alfc/options.ini");

	gdata->optfile = INI_load(gdata->optfilename);
	if(gdata->optfile == NULL)
	{
		gdata->optfile = INI_EmptyINF();
		CreateBaselineINIFile(gdata);
	}

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
}

void SaveOptions(uGlobalData *gdata)
{
	CreateHomeDirectory();
	INI_save(gdata->optfilename, gdata->optfile);
}
