/****h* ALFC/LuaAPICommon
 * FUNCTION
 *   The C<>Lua interface functions used in the Lua scripts.
 *   This is the common functions can are shared between the directory mode, viewer etc.
 *****
 */

#include "headers.h"

static int HaveShellMetaCharacters(char *s)
{
	while (*s != 0)
	{
		switch (*s++)
		{
			case '|':
			case '^':
			case ';':
			case '&':
			case '(':
			case ')':
			case '<':
			case '>':
			case '[':
			case ']':
			case '*':
			case '?':
			case '\'':
			case '\"':
			case '\\':
			case '`':
			case '$':
			case '~':
				return 1;
		}
	}

	return 0;
}

/****f* LuaAPICommon/VersionDate
 * FUNCTION
 *	Returns the build date
 * SYNOPSIS
 dte = VersionDate()
 * INPUTS
 *	o None
 * RESULTS
 *	o dte (string) -- C __DATE__ stamp as string of build
 * EXAMPLE
 debug_msg("ALFC built on " .. VersionDate())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionDate(lua_State *L)
{
	lua_pushstring(L, VersionDate());
	return 1;
}

/****f* LuaAPICommon/VersionTime
 * FUNCTION
 *	Returns the build time
 * SYNOPSIS
 dte = VersionTime()
 * INPUTS
 *	o None
 * RESULTS
 *	o dte (string) -- C __TIME__ stamp as string of build
 * EXAMPLE
 debug_msg("ALFC built on " .. VersionTime())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionTime(lua_State *L)
{
	lua_pushstring(L, VersionTime());
	return 1;
}

/****f* LuaAPICommon/VersionMajor
 * FUNCTION
 *	Returns the major version number
 * SYNOPSIS
 major = VersionMajor()
 * INPUTS
 *	o None
 * RESULTS
 *	o major (number) -- major number (eg: 1)
 * EXAMPLE
 debug_msg("ALFC v " .. VersionMajor())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionMajor(lua_State *L)
{
	lua_pushnumber(L, VersionMajor());
	return 1;
}

/****f* LuaAPICommon/VersionMinor
 * FUNCTION
 *	Returns the minor version number
 * SYNOPSIS
 minor = VersionMinor()
 * INPUTS
 *	o None
 * RESULTS
 *	o minor (number) -- minor number (eg: 2)
 * EXAMPLE
 debug_msg("ALFC minor version = " .. VersionMinor())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionMinor(lua_State *L)
{
	lua_pushnumber(L, VersionMinor());
	return 1;
}

/****f* LuaAPICommon/VersionBuild
 * FUNCTION
 *	Returns the build number
 * SYNOPSIS
 build = VersionBuild()
 * INPUTS
 *	o None
 * RESULTS
 *	o build (number) -- build number (eg: 1234)
 * EXAMPLE
 debug_msg("ALFC build " .. VersionBuild())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionBuild(lua_State *L)
{
	lua_pushnumber(L, VersionBuild());
	return 1;
}

/****f* LuaAPICommon/Version
 * FUNCTION
 *	This function returns the version in a hexadecimal encoding. The upper 8 bits
 *   are the major number, the next 8 bits are minor and the lower 16 are build.
 *    0-15 - Build
 *   16-23 - Minor
 *   24-31 - Major
 * SYNOPSIS
 vers = Version()
 * INPUTS
 *	o None
 * RESULTS
 *	o version (number) -- version number (eg: 1.02.1234 as 0x010204D2)
 * EXAMPLE
 debug_msg("ALFC version " .. Version())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_Version(lua_State *L)
{
	lua_pushnumber(L, (VersionMajor() << 24) | (VersionMinor() << 16) | VersionBuild());
	return 1;
}

/****f* LuaAPICommon/GetMode
 * FUNCTION
 *	Returns the current mode the program is running in
 * SYNOPSIS
 mode = GetMode()
 * INPUTS
 *	o None
 * RESULTS
 *	o mode (number) -- Returns currently active mode.
 *	mode:
 *	o 1 = eMode_Directory
 *	o 2 = eMode_Viewer
 * EXAMPLE
 if GetMode() == eMode_Viewer then
	 debug_msg("Script is running inside the viewer")
 end
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
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

	if (*s == 0x0)
		return strdup("");

	x = strdup(s);

	p = strchr(x, 0x0);
	p -= 1;

	while (p > x && isspace(*p) != 0)
		p--;

	*(p + 1) = 0;

	p = strdup(x);
	free(x);

	return p;
}

static char* ltrim(const char *s)
{
	const char *p;

	p = s;
	while (*p != 0x0 && isspace(*p) != 0)
		p++;

	return strdup(p);
}

/****f* LuaAPICommon/trim
 * FUNCTION
 *	Trims both trailing and leading spaces
 * SYNOPSIS
 string = trim(string)
 * INPUTS
 *	o string
 * RESULTS
 *	o string
 * SEE ALSO
 * rtrim, ltrim
 * AUTHOR
 *	Stu George
 ******
 */
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

/****f* LuaAPICommon/ltrim
 * FUNCTION
 *	Trims only leading spaces
 * SYNOPSIS
 string = ltrim(string)
 * INPUTS
 *	o string
 * RESULTS
 *	o string
 * SEE ALSO
 * rtrim, trim
 * AUTHOR
 *	Stu George
 ******
 */
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

/****f* LuaAPICommon/rtrim
 * FUNCTION
 *	Trims only trailing spaces
 * SYNOPSIS
 string = rtrim(string)
 * INPUTS
 *	o string
 * RESULTS
 *	o string
 * SEE ALSO
 * ltrim, trim
 * AUTHOR
 *	Stu George
 ******
 */
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

/****f* LuaAPICommon/debug_msg
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

/****f* LuaAPICommon/VersionString
 * FUNCTION
 *	This function will return a string representation
 *	of the program version in the format of %i.%02i/%04i of Major Version,
 *	Minor Version and Build Number.
 * SYNOPSIS
 vers = VersionString()
 * INPUTS
 *	o None
 * RESULTS
 *	o version (string) -- version number (eg: 1.02.1234)
 * EXAMPLE
 debug_msg("ALFC version " .. VersionString())
 * SEE ALSO
 * VersionDate, VersionTime, Version, VersionMinor, VersionMajor, VersionBuild, VersionString
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_VersionString(lua_State *L)
{
	char *str = malloc(64);

	sprintf(str, "%i.%02i/%04i", VersionMajor(), VersionMinor(), VersionBuild());
	lua_pushstring(L, str);
	free(str);

	return 1;
}

/****f* LuaAPICommon/ConvertDirectoryName
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

/****f* LuaAPICommon/DriverName
 * FUNCTION
 *	returns a string for the current output driver
 * SYNOPSIS
 string = DriverName()
 * INPUTS
 *	o string
 * RESULTS
 *	o string
 *	driver:
 *	o "GUI" - Basically the gui driver on both X11 and Win32.
 *	o "NCURSES" - NCurses text mode driver
 * AUTHOR
 *	Stu George
 * SEE ALSO
 * SystemType
 ******
 */
int gmec_DriverName(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushstring(L, gd->screen->driver_name());
	return 1;
}

/****f* LuaAPICommon/IncludeFile
 * FUNCTION
 *	This function includes another lua script inside the current one.
 *	This The included file is then executed to run any global code/definitions in side it.
 * SYNOPSIS
 IncludeFile("$ALFC/filename.lua")
 * INPUTS
 *	o filename (string) - File to load
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_IncludeFile(lua_State *L)
{
	struct lstr s;
	char *cpath;
	uint8_t *buff;
	int buffsize;
	int v;
	FILE *fp;

	GET_LUA_STRING(s, 1);

	cpath = ConvertDirectoryName(s.data);

	fp = fopen(cpath, "rb");
	if (fp == NULL)
	{
		free(cpath);
		return luaL_error(L, "Could not open file %s\n", s.data);
	}

	fseek(fp, 0x0L, SEEK_END);
	buffsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	buff = calloc(1, buffsize + 32);
	if (buff == NULL)
	{
		free(cpath);
		fclose(fp);
		return luaL_error(L, "Not enough memory to load file %s\n", s.data);
	}

	fread(buff, 1, buffsize, fp);
	fclose(fp);
	free(cpath);

	v = luaL_loadbuffer(L, (char*) buff, buffsize, s.data);
	if (v != 0)
	{
		LogError("cant include (%s) lua error : %s", s.data, lua_tostring(L, -1));
	}
	else
	{
		v = lua_pcall(L, 0, 0, 0); // call 'SetGlobals' with 0 arguments and 0 result
		if (v != 0)
		{
			LogError("include (%s) lua error : %s", s.data, lua_tostring(L, -1));
		}
	}

	free(buff);

	return 0;
}

/****f* LuaAPICommon/DoesFileExist
 * FUNCTION
 *	This function tests for the existance of a file
 * SYNOPSIS
 x = DoesFileExist("$ALFC/global.lua")
 * INPUTS
 *	o filename (string) - file to test
 * RESULTS
 *	o result (integer)
 *	result:
 *	o 0 - yes file exists
 *	o -1 - no file does not exist
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_DoesFileExist(lua_State *L)
{
	char *dir;
	struct lstr d;
	struct stat buff;

	GET_LUA_STRING(d, 1);
	dir = ConvertDirectoryName(d.data);

	if (ALFC_stat(dir, &buff) == 0)
		lua_pushnumber(L, 0);
	else
		lua_pushnumber(L, -1);

	free(dir);
	return 1;
}

void push_file(lua_State *L, uDirEntry *de, int idx, char *path)
{
	char *buff_date;
	char date_fmt[8] =
	{
	et_Year4, et_Month, et_Day, ' ', et_Hour24, et_Min, et_Sec, 0
	};

	lua_pushnumber(L, idx);
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, de->name);
	lua_settable(L, -3);

	lua_pushstring(L, "size");
	lua_pushnumber(L, de->size);
	lua_settable(L, -3);

	buff_date = GetDateTimeString(date_fmt, de->time);
	lua_pushstring(L, "date");
	lua_pushstring(L, buff_date);
	lua_settable(L, -3);
	free(buff_date);

	lua_pushstring(L, "link");
	if (de->lnk == NULL || strlen(de->lnk) == 0)
		lua_pushnumber(L, 0);
	else
		lua_pushnumber(L, 1);
	lua_settable(L, -3);

	lua_pushstring(L, "directory");
	if (ALFC_IsDir(de->attrs) == 0)
		lua_pushnumber(L, 1);
	else
		lua_pushnumber(L, 0);
	lua_settable(L, -3);

	lua_pushstring(L, "tagged");
	lua_pushnumber(L, de->tagged);
	lua_settable(L, -3);

	lua_pushstring(L, "path");
	lua_pushstring(L, path);
	lua_settable(L, -3);

	lua_settable(L, -3);
}

/****f* LuaAPICommon/GetFileListFromPath
 * FUNCTION
 *	This function tests for the existance of a file
 * SYNOPSIS
 x = GetFileListFromPath("$HOME")
 * INPUTS
 *	o directory (string) - directory to enumerate files from
 * RESULTS
 *	o filelist (table) - a table of data
 *	data:
 *	o name (string) - filename
 *	o size (number) - size of file
 *	o date (string) - date time formatted string per preferences
 *	o link (number) - if file is a symlink or not
 *	o directory (number) - if file is directory or not
 *	o tagged (number) - if tagged or not
 *	o path (string) - path to filename
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_GetFileListFromPath(lua_State *L)
{
	DList *lst;
	DLElement *e;
	struct lstr d;
	uGlobalData *gd;
	uDirEntry *de;
	char *dir;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(d, 1);

	dir = ConvertDirectoryName(d.data);
	lst = GetFiles(gd, dir);

	if (lst != NULL)
	{
		int i;

		lua_newtable(L);

		i = 1;
		e = dlist_head(lst);
		while (e != NULL)
		{
			de = dlist_data(e);

			push_file(L, de, i++, dir);

			e = dlist_next(e);
		}
	}

	free(dir);

	dlist_destroy(lst);
	free(lst);

	return 1;
}

/****f* LuaAPICommon/SystemType
 * FUNCTION
 *	returns a string for the current system (windows, unix, etc)
 * SYNOPSIS
 string = SystemType()
 * INPUTS
 *	o string
 * RESULTS
 *	o string
 *	driver:
 *	o "WIN32" - Windows machines
 *	o "UNIX" - Unix boxes
 * AUTHOR
 *	Stu George
 * SEE ALSO
 * DriverName
 ******
 */
int gmec_SystemType(lua_State *L)
{
#ifdef __WIN32__
	lua_pushstring(L, "WIN32");
#else
	lua_pushstring(L, "UNIX");
#endif
	return 1;
}

/****f* LuaAPICommon/globmatch
 * FUNCTION
 *	tests a glob against a string
 * SYNOPSIS
 result = globmatch(string, globpattern)
 * INPUTS
 *	o string to test for pattern
 *	o pattern
 * RESULTS
 *	o value (number)
 *	value:
 *	o 0 - matches
 *	o -1 - does not match
 * EXAMPLE
 if globmatch("readme.txt", "*.txt") == 1 then
	 debug_msg("matches *.txt pattern")
 end
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_globmatch(lua_State *L)
{
	struct lstr name;
	struct lstr pattern;
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(name, 1);
	GET_LUA_STRING(pattern, 2);

	if (fnmatch(pattern.data, name.data, 0) == 0)
		lua_pushnumber(L, 0);
	else
		lua_pushnumber(L, -1);

	return 1;
}


/****f* LuaAPICommon/exec
 * FUNCTION
 *	Executes an external program.
 *	Program will determine shell meta characters and execute $SHELL with the parameters.
 * SYNOPSIS
 exec("/usr/bin/ls")
 * INPUTS
 *	o filename to exec
 * RESULTS
 *	o None
 * EXAMPLE
 exec("/bin/cat $HOME/test.txt | sort")
 * PORTABILITY
 *  On windows COMSPEC environment variable will be used for shell, on unix SHELL will be invoked.
 * AUTHOR
 *	Stu George
 ******
 */
int gmec_exec(lua_State *L)
{
	struct lstr cmd;
	int meta;
	int rc;

	char *path;

	rc = 0;

	GET_LUA_STRING(cmd, 1);
	meta = HaveShellMetaCharacters(cmd.data);

	if (meta == 1)
	{
		// exec with sh
		char *shell;

		shell = getenv("SHELL");
		if (shell != NULL)
		{

		}
	}
	else
	{
		// exec binary?

		path = getenv("PATH");
		if (path == NULL)
			path = ALFC_get_basepath();


		// split it with ALFC_path_varset
		// run exec on name in each path if it does not contain path in its name
		// until we hit a good one
	}

	lua_pushnumber(L, rc);
	return 1;
}

