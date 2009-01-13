#include "headers.h"

#define GLOBALDATA "uGlobalData"

static const char *uGlobalData_Key = "uGlobalData";
uGlobalData* GetGlobalData(lua_State *L)
{
	uGlobalData *gb;

	lua_pushlightuserdata(L, (void *)&uGlobalData_Key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	gb = (uGlobalData*)lua_touserdata(L, -1);

	return gb;
}

int RegisterGlobalData(uGlobalData *gb, lua_State *l)
{
	lua_pushlightuserdata(l, (void*)&uGlobalData_Key);  /* push address */
	lua_pushlightuserdata(l, gb);
	lua_settable(l, LUA_REGISTRYINDEX);

	return 1;
}


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
	char *fn;

	GET_LUA_STRING(scr, 1);

	fn = ConvertDirectoryName(scr.data);
	lua_pushnumber(L, ExecuteScript(GetGlobalData(L), fn));
	free(fn);

	return 1;
}

// gets an option
// param: Group Name
// parma: Key Name
int gme_GetOption(lua_State *L)
{
	uGlobalData *gd;
	struct lstr group;
	struct lstr item;

	char *q;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(group, 1);
	GET_LUA_STRING(item, 2);

	q = INI_get(gd->optfile, group.data, item.data);
	lua_pushstring(L, q);

	return 1;
}

int gme_SetOption(lua_State *L)
{
	uGlobalData *gd;
	struct lstr group;
	struct lstr item;
	struct lstr val;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(group, 1);
	GET_LUA_STRING(item, 2);
	GET_LUA_STRING(val, 2);

	INI_UpdateItem(gd->optfile, group.data, item.data, val.data);

	return 0;
}

int gme_SaveOptions(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	CreateHomeDirectory();
	INI_save(gdata->optfilename, gdata->optfile);

	return 0;
}

