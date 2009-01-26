#include <windows.h>
#ifdef __MINGW_H
// needed for mkdir on mingw
#include <io.h>
#endif

#include "headers.h"

static char *alfc_script_home;

char* ALFC_getenv(const char *s)
{
	char *p;

	if(strcmp(s, "HOME") == 0)
	{
		p = getenv("HOME");
		if(p != NULL)
			return p;

		p = getenv("USERPROFILE");
		if(p != NULL)
			return p;

		return NULL;
	}
	else if(strcmp(s, "ALFC") == 0)
	{
		p = getenv("ALFC");
		if(p != NULL)
			return p;

		return alfc_script_home;
	}
	else
		return getenv(s);
}


void ALFC_GetUserInfo(uGlobalData *gd)
{
	char UserName[100];
	DWORD UserSize = 100;

	GetUserName(UserName, &UserSize);

	gd->strRealName = strdup(UserName);
	gd->strLoginName = strdup(UserName);
	gd->strHomeDirectory = strdup(getenv("USERPROFILE"));
	gd->strShell = strdup("cmd.exe");
	gd->uid = INT_MAX;
	gd->gid = INT_MAX;
}

uint64_t ALFC_GetFileSize(uDirEntry *de, struct stat *buff)
{
	WIN32_FIND_DATA fd;
	HANDLE err;
	uint64_t rc;

	if( S_ISDIR(buff->st_mode) != 0)
		return 0;

	err = FindFirstFile(de->name, &fd);

	if(err == INVALID_HANDLE_VALUE)
		return 0L;

	rc = fd.nFileSizeHigh & 0xFFFFFFFF;
	rc = rc << 32;
	rc |= fd.nFileSizeLow & 0xFFFFFFFF;

	FindClose(&fd);

	return rc;
}

uint32_t ALFC_GetFileAttrs(uDirEntry *de, struct stat *buff)
{
	return buff->st_mode;
}

time_t ALFC_GetFileTime(uDirEntry *de, struct stat *buff)
{
	return buff->st_mtime;
}

int ALFC_stat(char *fn, struct stat *buff)
{
	return stat(fn, buff);
}

int ALFC_mkdir(char *s)
{
	return mkdir(s);
}

int ALFC_rmdir(char *s)
{
	int rc = rmdir(s);
	if(rc != 0)
		LogInfo("rmdir : %s\n", strerror(errno));
	return rc;
}


int ALFC_startup(void)
{
	alfc_script_home = ConvertDirectoryName("$HOME/.alfc/scripts");

	return 0;
}

int ALFC_shutdown(void)
{
	if(alfc_script_home != NULL)
		free(alfc_script_home);
	return 0;
}
