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

	lua_settop(l, 0);
}

