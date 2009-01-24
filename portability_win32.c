#include <windows.h>
#include "headers.h"

char* ALFC_getenv(const char *s)
{
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

