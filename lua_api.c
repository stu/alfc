#include "headers.h"

#define GLOBALDATA "uGlobalData"

static uGlobalData *toGlobalData(lua_State *L, int index)
{
	uGlobalData *bar = (uGlobalData *)lua_touserdata(L, index);
	if (bar == NULL) luaL_typerror(L, index, GLOBALDATA);
	return bar;
}

uGlobalData *checkGlobalData(lua_State *L, int index)
{
	uGlobalData *bar;
	luaL_checktype(L, index, LUA_TUSERDATA);
	bar = (uGlobalData *)luaL_checkudata(L, index, GLOBALDATA);
	if (bar == NULL) luaL_typerror(L, index, GLOBALDATA);
	return bar;
}

uGlobalData *pushGlobalData(lua_State *L)
{
	uGlobalData *bar = (uGlobalData *)lua_newuserdata(L, sizeof(uGlobalData));
	luaL_getmetatable(L, GLOBALDATA);
	lua_setmetatable(L, -2);
	return bar;
}

static int uGlobalData_new (lua_State *L)
{
	uGlobalData *bar = pushGlobalData(L);
	assert(bar != NULL);

	return 1;
}

static const luaL_reg uGlobalData_methods[] = {
	{"new",           uGlobalData_new},

	{0, 0}
};

static int uGlobalData_gc (lua_State *L)
{
	return 0;
}

static int uGlobalData_tostring (lua_State *L)
{
	char buff[32];

	sprintf(buff, "%p", (void*)toGlobalData(L, 1));
	lua_pushfstring(L, GLOBALDATA " (%s)", buff);
	return 1;
}


static const luaL_reg uGlobalData_meta[] =
{
	{"__gc",       uGlobalData_gc},
	{"__tostring", uGlobalData_tostring},
	{0, 0}
};

int RegisterGlobalData(uGlobalData *gb, lua_State *l)
{
	uGlobalData *gx;

	luaL_openlib(l, GLOBALDATA, uGlobalData_methods, 0);
	luaL_newmetatable(l, GLOBALDATA);

	luaL_openlib(l, 0, uGlobalData_meta, 0);    /* fill metatable */
	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -3);               /* dup methods table*/
	lua_rawset(l, -3);                  /* metatable.__index = methods */
	lua_pushliteral(l, "__metatable");
	lua_pushvalue(l, -3);               /* dup methods table*/
	lua_rawset(l, -3);                  /* hide metatable: metatable.__metatable = methods */
	lua_pop(l, 1);                      /* drop metatable */


	gx = pushGlobalData(l);
	memmove(gx, gb, sizeof(uGlobalData));
	lua_setglobal(l, GLOBALDATA);

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
	uGlobalData *gd;
	char *fn;

	gd = checkGlobalData(L, 1);
	GET_LUA_STRING(scr, 2);

	fn = ConvertDirectoryName(scr.data);
	lua_pushnumber(L, ExecuteScript(gd, fn));
	free(fn);

	return 1;
}

// gets an option
// param: Group Name
// parma: Key Name
int gme_GetOption(lua_State *L)
{

	return 0;
}
