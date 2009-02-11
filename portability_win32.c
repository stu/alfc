#include <windows.h>
#include "headers.h"

static char *alfc_script_home;

static void GetWindowsErrorMsg(char *func)
{
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
	DWORD dw;

	dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)func) + 40) * sizeof(TCHAR));
	sprintf(lpDisplayBuf, "%s failed with error %d: %s", func, dw, lpMsgBuf);
	LogInfo(lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

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
	HANDLE hFind;
	uint64_t rc;

	if( S_ISDIR(buff->st_mode) != 0)
		return 0;

	hFind = FindFirstFile(de->name, &fd);

	if(hFind == INVALID_HANDLE_VALUE)
		return 0L;

	rc = fd.nFileSizeHigh & 0xFFFFFFFF;
	rc = rc << 32;
	rc |= fd.nFileSizeLow & 0xFFFFFFFF;

	FindClose(hFind);

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
	int rc;

	rc = CreateDirectory(s, NULL);

	if(rc == 0)
		rc = -1;
	else
		rc = 0;

	if(rc == -1)
	{
		GetWindowsErrorMsg(s);
	}

	return rc;
}

int ALFC_rmdir(char *s)
{
	int rc;

	rc = RemoveDirectory(s);

	// 0 = fail, non zero for success
	if(rc == 0)
		rc = -1;
	else
		rc = 0;

	if(rc == -1)
	{
		GetWindowsErrorMsg("rmdir");
	}

	return rc;
}

int ALFC_unlink(char *s)
{
	int rc;

	rc = DeleteFile(s);

	// 0 = fail, non zero for success
	if(rc == 0)
		rc = -1;
	else
		rc = 0;

	if(rc == -1)
	{
		GetWindowsErrorMsg("unlink");
	}

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
