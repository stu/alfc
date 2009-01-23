#include "headers.h"

int gmec_GetMode(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->mode);
	return 1;
}


static char* rtrim(const char *s)
{
	char *p;
	char *x;

	if(*s == 0x0)
		return strdup("");

	x = strdup(s);

	p = strchr(x, 0x0);
	p -= 1;

	while(p > x && isspace(*p) != 0 )
		p--;

	*(p+1)=0;

	p = strdup(x);
	free(x);

	return p;
}

static char* ltrim(const char *s)
{
	const char *p;

	p = s;
	while(*p != 0x0 && isspace(*p) != 0)
		p++;

	return strdup(p);
}

int gmec_trim(lua_State *L)
{
	struct lstr strx;
	char *a, *b;

	GET_LUA_STRING(strx, 1);

	a = ltrim(strx.data);
	b = rtrim(a);

	free(a);
	lua_pushstring(L, b);
	free(b);

	return 1;
}

int gmec_ltrim(lua_State *L)
{
	struct lstr strx;
	char *a;

	GET_LUA_STRING(strx, 1);

	a = ltrim(strx.data);
	lua_pushstring(L, a);
	free(a);
	return 1;
}

int gmec_rtrim(lua_State *L)
{
	struct lstr strx;
	char *a;

	GET_LUA_STRING(strx, 1);

	a = rtrim(strx.data);
	lua_pushstring(L, a);
	free(a);
	return 1;
}

/****f* LuaAPI/debug_msg
* FUNCTION
*	This outputs a string to the logging window
* SYNOPSIS
debug_msg(message)
* INPUTS
*	o Message (string) -- String to log
* RESULTS
*	o None
* EXAMPLE
debug_msg("Log this string to the logging window")
* AUTHOR
*	Stu George
******
*/
int gmec_debug_msg(lua_State *L)
{
	struct lstr msg;

	GET_LUA_STRING(msg, 1);

	LogInfo("%s\n", msg.data);

	return 0;
}


/****f* LuaAPI/GetVersionString
* FUNCTION
*	This function will return a string representation
*	of the program version in the format of %i.%02i/%04i of Major Version,
*	Minor Version and Build Number.
* SYNOPSIS
vers = GetVersionString()
* INPUTS
*	o None
* RESULTS
*	o version (string) -- version number (eg: 1.02.1234)
* EXAMPLE
debug_msg("ALFC version " .. GetVersionString())
* AUTHOR
*	Stu George
******
*/
int gmec_GetVersionString(lua_State *L)
{
	char *str = malloc(64);

	sprintf(str, "%i.%02i/%04i", VersionMajor(), VersionMinor(), VersionBuild());
	lua_pushstring(L, str);
	free(str);

	return 1;
}

/****f* LuaAPI/ConvertDirectoryName
* FUNCTION
*	This function will take a path and convert
*	environment strings into an absolute path name
*
* 	Will translate any Environment variable that begins with '$' and is
* 	delimited by '/' (or end of string) and will also substitute '~' for '$HOME'.
*
* SYNOPSIS
newPath = ConvertDirectoryName(Path)
* INPUTS
*	o Path -- The path to convert
* RESULTS
*	o newPath (string) -- Converted path
* EXAMPLE
debug_msg("$HOME == " .. ConvertDirectoryName("$HOME"))
* PORTABILITY
*	On windows machines if "$HOME" is present, it will be used,
*	but if it is not, it will substitute "$USERPROFILE"
* AUTHOR
*	Stu George
******
*/
int gmec_ConvertDirectoryName(lua_State *L)
{
	char *dir;
	struct lstr d;

	GET_LUA_STRING(d, 1);

	dir = ConvertDirectoryName(d.data);

	lua_pushstring(L, dir);
	free(dir);

	return 1;
}

static int ReadHistoryLine(uGlobalData *gd, int intLine, uint8_t *buff, int len)
{
	DLElement *e;
	int j;
	char *x;

	e = dlist_tail(gd->lstLogHistory);

	j = intLine;
	while( j > 0 && e != NULL)
	{
		e = dlist_prev(e);
		j -= 1;
	}

	if(j > 0 || e == NULL)
		return -1;

	x = dlist_data(e);
	if( strlen(x) >= len)
	{
		memmove(buff, x, len);
		buff[len-1] = 0;
	}
	else
	{
		memmove(buff, x, strlen(x) + 1);
	}

	return 0;
}

int gmec_ViewHistory(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	ViewFile(gd, "HISTORY BUFFER", &ReadHistoryLine);
	DrawAll(gd);

	return 0;
}

