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
	INI_save(gd->optfilename, gd->optfile);

	return 0;
}

int gme_GetCurrentWorkingDirectory(lua_State *L)
{
	char *x = GetCurrentWorkingDirectory();
	lua_pushstring(L, x);
	free(x);

	return 1;
}


int gme_ClearScreen(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	gd->screen->cls();

	return 0;
}

int gme_GetScreenHeight(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->screen->get_screen_height());

	return 1;
}

int gme_GetScreenWidth(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	lua_pushnumber(L, gd->screen->get_screen_width());

	return 1;
}

int gme_Print(lua_State *L)
{
	struct lstr msg;
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(msg, 1);

	gd->screen->print(msg.data);

	return 0;
}

int gme_GetUserID(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->uid);
	return 1;
}


int gme_GetUserGroup(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->gid);
	return 1;
}

int gme_SetColour(lua_State *L)
{
	//SetColour(title_background, "red")
	uGlobalData *gd;
	int style;
	struct lstr clr;
	uint32_t c;

	style = luaL_checknumber(L, 1);
	GET_LUA_STRING(clr, 2);

	gd = GetGlobalData(L);
	assert(gd != NULL);

	c = decode_colour(clr.data, -1);
	if(c == -1)
		return luaL_error(L, "Unknown colour %s", clr.data);

	switch(style)
	{
		case e_highlight_background:
			gd->clr_hi_background = c;
			gd->screen->init_style(STYLE_HIGHLIGHT, gd->clr_hi_foreground, gd->clr_hi_background);
			break;
		case e_highlight_foreground:
			gd->clr_hi_foreground = c;
			gd->screen->init_style(STYLE_HIGHLIGHT, gd->clr_hi_foreground, gd->clr_hi_background);
			break;
		case e_background:
			gd->clr_background = c;
			gd->screen->init_style(STYLE_NORMAL, gd->clr_foreground, gd->clr_background);
			break;
		case e_foreground:
			gd->clr_foreground = c;
			gd->screen->init_style(STYLE_NORMAL, gd->clr_foreground, gd->clr_background);
			break;
		case e_title_foreground:
			gd->clr_title_fg = c;
			gd->screen->init_style(STYLE_TITLE, gd->clr_title_fg, gd->clr_title_bg);
			break;
		case e_title_background:
			gd->clr_title_bg = c;
			gd->screen->init_style(STYLE_TITLE, gd->clr_title_fg, gd->clr_title_bg);
			break;
	}

	return 0;
}

int gme_GetRuntimeOption_CompressFilesize(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->compress_filesize);
	return 1;
}

int gme_SetRuntimeOption_CompressFilesize(lua_State *L)
{
	uGlobalData *gd;
	int v;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = luaL_checknumber(L, 1);
	if(v == 1)
		gd->compress_filesize = 1;
	else
		gd->compress_filesize = 0;

	return 0;
}