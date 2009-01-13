#include <sys/stat.h>
#include <sys/types.h>

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

	q = ConvertDirectoryName("$HOME/.alfm");

	mkdir(q, 0744);

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

	INI_UpdateItem(gdata->optfile, "scripts", "startup_file", "$HOME/.alfm/startup.lua");
	INI_UpdateItem(gdata->optfile, "scripts", "shutdown_file", "$HOME/.alfm/shutdown.lua");
}

void LoadOptions(uGlobalData *gdata)
{
	gdata->optfilename = ConvertDirectoryName("$HOME/.alfm/options.ini");
	gdata->optfile = INI_load(gdata->optfilename);
	if(gdata->optfile == NULL)
	{
		gdata->optfile = INI_EmptyINF();
		CreateBaselineINIFile(gdata);
	}
}

void SaveOptions(uGlobalData *gdata)
{
	CreateHomeDirectory();
	INI_save(gdata->optfilename, gdata->optfile);
}
