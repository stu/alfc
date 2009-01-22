#include "headers.h"

int gme_GetMode(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->mode);
	return 1;
}

