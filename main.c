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
	return realloc(p, 4 + strlen(p));
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
		LoadOptions(gdata);


		ExecStartupScript(gdata);



		ExecShutdownScript(gdata);

		SaveOptions(gdata);

		free(gdata->optfilename);
		free(gdata);
	}

	LogWrite_Shutdown();

	return 0;
}

