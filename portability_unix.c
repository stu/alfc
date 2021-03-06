#ifdef BUILD_UNIXLIKE
#include "headers.h"
#include <pwd.h>

#ifdef xlib
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#endif

const char *ALFC_str_pathsep = "/";
const char ALFC_pathsep = '/';
const char ALFC_path_varset = ':';

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


uint64_t ALFC_GetFileSize(uDirEntry *de)
{
	return de->size;
}

uint32_t ALFC_GetFileAttrs(uDirEntry *de)
{
	return de->attrs;
}

time_t ALFC_GetFileTime(uDirEntry *de)
{
	return de->time;
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
	char *x;
	char *z;
	struct stat dbuff;
	struct stat pbuff;
	int rc;
	char *p;


	p = strchr(s, '/');
	if(p == NULL)
	{
		z = calloc(1, 1024);
		getcwd(z, 1023);
		strcat(z, "/");
		strcat(z, s);
	}
	else
		z = s;

	x = strdup(z);

	p = strchr(x, 0);
	while(p != x && *p != '/')
		p--;

	if(p > x)
		*p = 0;

	ALFC_stat(x, &pbuff);
	ALFC_stat(s, &dbuff);

	chmod(x, pbuff.st_mode | S_IWUSR | S_IWGRP | S_IWOTH | S_IRGRP | S_IROTH | S_IRUSR);
	chmod(s, dbuff.st_mode | S_IWUSR | S_IWGRP | S_IWOTH | S_IRGRP | S_IROTH | S_IRUSR);

	rc = rmdir(s);

	chmod(x, pbuff.st_mode);

	if(rc != 0)
		chmod(s, dbuff.st_mode);

	free(x);

	if(z != s)
		free(z);

	return rc;
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
	// SGEO 20090808 - switch unlink to remove to handle dangling symlinks
	return remove(s);
}

char* ALFC_get_last_error(int err)
{
	return strerror(errno);
}

int ALFC_IsHidden(char *fn, uint32_t attrs)
{
	if(fn[0] == '.' && (fn[1] != 0 && fn[1] != '.'))
		return 0;
	else
		return -1;
}

int ALFC_IsExec(char *fn, uint32_t attrs)
{
	if( (attrs & (S_IXUSR | S_IXOTH | S_IXGRP)) != 0)
		return 0;
	else
		return -1;
}

int ALFC_IsDir(uint32_t attrs)
{
	if(S_ISDIR(attrs&S_IFDIR) != 0)
		return 0;
	else
		return -1;
}

char* ALFC_get_basepath(void)
{
	return strdup(":/bin:/usr/bin:/usr/local/bin:/");
}
#endif

