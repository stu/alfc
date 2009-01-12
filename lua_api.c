#include "headers.h"

int gme_debug_msg(lua_State *L)
{
	return 0;
}

int gme_GetVersionString(lua_State *L)
{
	char *str = malloc(64);

	sprintf(str, "v%i.%i.%04i", VersionMajor(), VersionMinor(), VersionBuild());

	lua_pushstring(L, str);
	return 1;
}

