#include "headers.h"

typedef struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

} uGlobalData;

static uGlobalData* NewGlobalData(void)
{
	return calloc(1, sizeof(uGlobalData));
}

/* Convert environment variables to actual strings */
static char* ConvertDirectoryName(const char *x)
{
	char *env;

	char *a;
	char *p;
	char *q;

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

			env = getenv(q);
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

	free(a);
	return realloc(p, 4 + strlen(p));
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

static void ExecStartupScript(uGlobalData *gdata)
{
	char *fn;
	char *cfn;

	fn = INI_get(gdata->optfile, "scripts", "startup_file");
	if(fn != NULL)
	{
		cfn = ConvertDirectoryName(fn);
		ExecuteScript(cfn);
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
		ExecuteScript(cfn);
		free(cfn);
	}
}

int main(int argc, char *argv[])
{
	uGlobalData *gdata;

	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR);

	LogInfo("" LUA_RELEASE "\n");
	LogInfo("" LUA_COPYRIGHT "\n");
	LogInfo("" LUA_AUTHORS "\n");


	gdata = NewGlobalData();

	if(gdata != NULL)
	{
		gdata->optfilename = ConvertDirectoryName("$HOME/.alfm/options.ini");
		gdata->optfile = INI_load(gdata->optfilename);
		if(gdata->optfile == NULL)
		{
			gdata->optfile = INI_EmptyINF();
			CreateBaselineINIFile(gdata);
		}

		ExecStartupScript(gdata);



		ExecShutdownScript(gdata);
		INI_save(gdata->optfilename, gdata->optfile);
		INI_unload(gdata->optfile);

		free(gdata->optfilename);
		free(gdata);
	}

	LogWrite_Shutdown();

	return 0;
}

