#include "headers.h"

// prints a debug message
int gme_debug_msg(lua_State *L)
{
	struct lstr msg;

	GET_LUA_STRING(msg, 1);

	LogInfo("%s\n", msg.data);

	return 0;
}

// returns version
// no params
int gme_GetVersionString(lua_State *L)
{
	char *str = malloc(64);

	sprintf(str, "%i.%i/%04i", VersionMajor(), VersionMinor(), VersionBuild());
	lua_pushstring(L, str);
	free(str);

	return 1;
}

// Converts a path environment vars, like $HOME/foo
// param: path
int gme_ConvertDirectoryName(lua_State *L)
{
	char *dir;
	struct lstr d;

	GET_LUA_STRING(d, 1);

	dir = ConvertDirectoryName(d.data);

	lua_pushstring(L, dir);
	free(dir);

	return 1;
}

// Executes a lua script
// param: scriptfile
int gme_ExecuteScript(lua_State *L)
{
	struct lstr scr;

	GET_LUA_STRING(scr, 1);

	lua_pushnumber(L, ExecuteScript(scr.data));

	return 1;
}
