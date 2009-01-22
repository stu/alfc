#include "headers.h"


/* register lua funcs */
void RegisterViewerFuncs(uViewFile *gb, lua_State *l)
{
	luaL_openlibs(l);

	RegisterCommonFuncs(gb->gd, l);
	RegisterGlobalData(gb->gd, l);
	RegisterViewerData(gb, l);


/* LUA EXT REF */
/* LUA END EXT REF */

	lua_pushnumber(l, VMAJ); lua_setglobal(l, "VERSION_MAJ");
	lua_pushnumber(l, VMIN); lua_setglobal(l, "VERSION_MIN");
	lua_pushnumber(l, VBUILD); lua_setglobal(l, "VERSION_BUILD");

	// colour setting constants
	lua_pushnumber(l, e_title_foreground); lua_setglobal(l, "title_foreground");
	lua_pushnumber(l, e_title_background); lua_setglobal(l, "title_background");
	lua_pushnumber(l, e_background); lua_setglobal(l, "background");
	lua_pushnumber(l, e_foreground); lua_setglobal(l, "foreground");
	lua_pushnumber(l, e_highlight_background); lua_setglobal(l, "highlight_background");
	lua_pushnumber(l, e_highlight_foreground); lua_setglobal(l, "highlight_foreground");

	lua_pushnumber(l, ALFC_KEY_DOWN); lua_setglobal(l, "ALFC_KEY_DOWN");
	lua_pushnumber(l, ALFC_KEY_UP); lua_setglobal(l, "ALFC_KEY_UP");
	lua_pushnumber(l, ALFC_KEY_LEFT); lua_setglobal(l, "ALFC_KEY_LEFT");
	lua_pushnumber(l, ALFC_KEY_RIGHT); lua_setglobal(l, "ALFC_KEY_RIGHT");
	lua_pushnumber(l, ALFC_KEY_INS); lua_setglobal(l, "ALFC_KEY_INS");
	lua_pushnumber(l, ALFC_KEY_DEL); lua_setglobal(l, "ALFC_KEY_DEL");
	lua_pushnumber(l, ALFC_KEY_BACKSPACE); lua_setglobal(l, "ALFC_KEY_BACKSPACE");
	lua_pushnumber(l, ALFC_KEY_HOME); lua_setglobal(l, "ALFC_KEY_HOME");
	lua_pushnumber(l, ALFC_KEY_END); lua_setglobal(l, "ALFC_KEY_END");
	lua_pushnumber(l, ALFC_KEY_PAGE_UP); lua_setglobal(l, "ALFC_KEY_PAGE_UP");
	lua_pushnumber(l, ALFC_KEY_PAGE_DOWN); lua_setglobal(l, "ALFC_KEY_PAGE_DOWN");
	lua_pushnumber(l, ALFC_KEY_ENTER); lua_setglobal(l, "ALFC_KEY_ENTER");
	lua_pushnumber(l, ALFC_KEY_TAB); lua_setglobal(l, "ALFC_KEY_TAB");
	lua_pushnumber(l, ALFC_KEY_SPACE); lua_setglobal(l, "ALFC_KEY_SPACE");
	lua_pushnumber(l, ALFC_KEY_F00); lua_setglobal(l, "ALFC_KEY_F00");
	lua_pushnumber(l, ALFC_KEY_F01); lua_setglobal(l, "ALFC_KEY_F01");
	lua_pushnumber(l, ALFC_KEY_F02); lua_setglobal(l, "ALFC_KEY_F02");
	lua_pushnumber(l, ALFC_KEY_F03); lua_setglobal(l, "ALFC_KEY_F03");
	lua_pushnumber(l, ALFC_KEY_F04); lua_setglobal(l, "ALFC_KEY_F04");
	lua_pushnumber(l, ALFC_KEY_F05); lua_setglobal(l, "ALFC_KEY_F05");
	lua_pushnumber(l, ALFC_KEY_F06); lua_setglobal(l, "ALFC_KEY_F06");
	lua_pushnumber(l, ALFC_KEY_F07); lua_setglobal(l, "ALFC_KEY_F07");
	lua_pushnumber(l, ALFC_KEY_F08); lua_setglobal(l, "ALFC_KEY_F08");
	lua_pushnumber(l, ALFC_KEY_F09); lua_setglobal(l, "ALFC_KEY_F09");
	lua_pushnumber(l, ALFC_KEY_F10); lua_setglobal(l, "ALFC_KEY_F10");
	lua_pushnumber(l, ALFC_KEY_F11); lua_setglobal(l, "ALFC_KEY_F11");
	lua_pushnumber(l, ALFC_KEY_F12); lua_setglobal(l, "ALFC_KEY_F12");

	lua_pushnumber(l, eMode_Directory); lua_setglobal(l, "eMode_Directory");
	lua_pushnumber(l, eMode_Viewer); lua_setglobal(l, "eMode_Viewer");

	lua_settop(l, 0);
}

