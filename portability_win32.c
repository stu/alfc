#include <windows.h>
#include "headers.h"

static char *alfc_script_home;
static char *last_error;

const char ALFC_pathsep = '\\';

static void GetWindowsErrorMsg(char *func)
{
    LPVOID lpMsgBuf;
	DWORD dw;

	dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

	last_error = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)func) + 40) * sizeof(TCHAR));
	sprintf(last_error, "%s failed with error %d: %s", func, dw, lpMsgBuf);
	LogInfo(last_error);
	LocalFree(lpMsgBuf);
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

	// remove any hidden/readonly attributes etc before deletion
	SetFileAttributes(s, FILE_ATTRIBUTE_NORMAL);

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
	last_error = NULL;
	alfc_script_home = ConvertDirectoryName("$HOME/.alfc/scripts");

	return 0;
}

int ALFC_shutdown(void)
{
	if(last_error != NULL)
		LocalFree(last_error);

	if(alfc_script_home != NULL)
		free(alfc_script_home);
	return 0;
}

char* ALFC_get_last_error(int err)
{
	return last_error;
}

void ALFC_GetScreenDimensions(int *w, int *h)
{
	DEVMODE dvmdOrig;
	HDC hdc = GetDC(NULL);

	dvmdOrig.dmPelsWidth = GetDeviceCaps(hdc, HORZRES);
	dvmdOrig.dmPelsHeight = GetDeviceCaps(hdc, VERTRES);
	dvmdOrig.dmBitsPerPel = GetDeviceCaps(hdc, BITSPIXEL);
	dvmdOrig.dmDisplayFrequency = GetDeviceCaps(hdc, VREFRESH);
	ReleaseDC(NULL, hdc);

	*h = dvmdOrig.dmPelsHeight;
	*w = dvmdOrig.dmPelsWidth;
}
