#include "headers.h"
#include <pwd.h>


char* ALFC_getenv(const char *s)
{
	return getenv(s);
}

void ALFC_GetUserInfo(uGlobalData *gd)
{
	struct passwd *passwd;
	char *x;
	char *p;

	passwd = getpwuid(getuid());

	x = strdup(passwd->pw_gecos);
	p = strchr(x, 0x0);
	p--;

	while(p>x)
	{
		if(*p == ',' && *(p+1)==0) *p = 0x0;
		if(*p == ' ' && *(p+1)==0) *p = 0x0;

		p--;
	}

	gd->strRealName = strdup(x);
	gd->strLoginName = strdup(passwd->pw_name);
	gd->strHomeDirectory = strdup(passwd->pw_dir);
	gd->strShell = strdup(passwd->pw_shell);
	gd->uid = getpwuid(getuid())->pw_uid;
	gd->gid = getpwuid(getuid())->pw_gid;

	free(x);
}


uint64_t ALFC_GetFileSize(uDirEntry *de, struct stat *buff)
{
	return buff->st_size;
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
	return lstat(fn, buff);
}

int ALFC_mkdir(char *s)
{
	return mkdir(s, 0744);
}

int ALFC_rmdir(char *s)
{
	return rmdir(s);
}

int ALFC_startup(void)
{
	return 0;
}

int ALFC_shutdown(void)
{
	return 0;
}

int ALFC_unlink(char *s)
{
	return unlink(s);
}

char* ALFC_get_last_error(int err)
{
	return strerror(errno);
}
