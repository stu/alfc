#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
 #include <errno.h>

#include "headers.h"

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


struct udtDirEntry
{
	char *name;
	struct stat stat_buff;
};
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
			struct udtDirEntry *de;

			dr = readdir(d);
			while(dr != NULL)
			{
				de = malloc(sizeof(struct udtDirEntry));
				memset(de, 0x0, sizeof(struct udtDirEntry));

				de->name = strdup(dr->d_name);
				lstat(de->name, &de->stat_buff);

				dlist_ins(lst, de);
				LogInfo("got %i - %s\n", de->stat_buff.st_size, de->name);

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

int main(int argc, char *argv[])
{
	uGlobalData *gdata;

	LogWrite_Startup(0, LOG_INFO | LOG_DEBUG | LOG_ERROR);

	LogInfo("" LUA_RELEASE "\n");
	LogInfo("" LUA_COPYRIGHT "\n");
	LogInfo("" LUA_AUTHORS "\n");


	gdata = NewGlobalData();
	SetupStartDirectories(gdata);

	if(gdata != NULL)
	{
		DList *lst;

		LoadOptions(gdata);


		ExecStartupScript(gdata);

		free(gdata->left_dir);
		gdata->left_dir = GetOptionDir(gdata, "left_startup", "mru_left");

		free(gdata->right_dir);
		gdata->right_dir = GetOptionDir(gdata, "right_startup", "mru_right");

		LogInfo("Start in left : %s\n", gdata->left_dir);
		LogInfo("Start in right : %s\n", gdata->right_dir);

		lst = GetFiles(gdata, gdata->left_dir);
		dlist_destroy(lst);
		free(lst);

		ExecShutdownScript(gdata);


		SaveOptions(gdata);
		INI_unload(gdata->optfile);

		chdir(gdata->startup_path);

		free(gdata->startup_path);
		free(gdata->left_dir);
		free(gdata->right_dir);

		free(gdata->optfilename);
		free(gdata);
	}

	LogWrite_Shutdown();

	return 0;
}

