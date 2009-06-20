#include <windows.h>
#include "headers.h"

static char *alfc_script_home;
static char *last_error;

const char *ALFC_str_pathsep = "\\";
const char ALFC_pathsep = '\\';
const char ALFC_path_varset = ';';

static void GetWindowsErrorMsg(char *func)
{
	LPVOID lpMsgBuf;
	DWORD dw;

	dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

	last_error = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)func) + 40) * sizeof(TCHAR));
	sprintf(last_error, "%s failed with error %d: %s", func, dw, lpMsgBuf);
	//LogInfo(last_error);
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
	gd->strShell = strdup(getenv("COMSPEC"));
	gd->uid = INT_MAX;
	gd->gid = INT_MAX;
}

uint64_t ALFC_GetFileSize(uDirEntry *de)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	uint64_t rc;
	char *x;

	if(ALFC_IsDir(de->attrs) == 0)
		return 0;

	x = malloc(strlen(de->path) + strlen(de->name) + 16);
	strcpy(x, de->path);
	strcat(x, "/");
	strcat(x, de->name);

	hFind = FindFirstFile(x, &fd);
	free(x);

	if(hFind == INVALID_HANDLE_VALUE)
		return 0L;

	rc = fd.nFileSizeHigh & 0xFFFFFFFF;
	rc = rc << 32;
	rc |= fd.nFileSizeLow & 0xFFFFFFFF;

	FindClose(hFind);

	return rc;
}

uint32_t ALFC_GetFileAttrs(uDirEntry *de)
{
	char *x;
	uint32_t z;

	x = malloc(strlen(de->path) + strlen(de->name) + 16);
	strcpy(x, de->path);
	strcat(x, "/");
	strcat(x, de->name);

	z = GetFileAttributes(x);
	if(z == 0xFFFFFFFF)
	{
		// HACK : SGEO : special case to handle the two locked files...
		// My test system shows default of AHS flags.
		if(strcasecmp(de->name, "pagefile.sys") == 0)
			z = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE;

		if(strcasecmp(de->name, "hiberfil.sys") == 0)
			z = FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE;
	}

	if(de->name[0] == '.')
		z |= FILE_ATTRIBUTE_HIDDEN;

	free(x);

	return z;

}

time_t ALFC_GetFileTime(uDirEntry *de)
{
	return de->time;
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

int ALFC_IsHidden(char *fn, uint32_t attrs)
{
	if( (attrs & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
		return 0;
	else
		return -1;
}

int ALFC_IsExec(char *fn, uint32_t attrs)
{
	return -1;
}

int ALFC_IsDir(uint32_t attrs)
{
	if( (attrs&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		return 0;
	else
		return -1;
}

char* ALFC_get_basepath(void)
{
	// SYSTEMROOT on XP usually "c:\windows" (mostly nt derivatives)
	// WINDIR usually c:\windows (moslty win95/95 derivatives)

	char *p;
	char buff[512];

	buff[0] = 0;

	// try systemroot first, then windir second

	p = getenv("SYSTEMROOT");
	if(p == NULL)
		p = getenv("WINDIR");

	if(p != NULL)
	{
		strcat(buff, p);
		strcat(buff, ";");
		strcat(buff, p);
		strcat(buff, "\\system32");
		strcat(buff, ";");
		// for good measure on systems without a system32
		strcat(buff, "\\system");
		strcat(buff, ";");
	}
	else
		strcat(buff, "C:\\Winnt;C:\\Winnt\\System32;c:\\windows;c:\\windows\\system32");

	return strdup(buff);
}

