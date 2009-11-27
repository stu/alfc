#include "headers.h"

int ExecuteGlobalViewerString(uViewFile *gb, char *sn)
{
	char *outbuff;
	int maxsize;
	int rc = 0;

	lua_State *l = gb->_GL;
	int v;

	maxsize = strlen(sn);
	outbuff = malloc(256 + maxsize);

	if(outbuff == NULL)
	{
		LogError("No memory for buffer\n");
		return -1;
	}

	sprintf(outbuff, "local function GLOBAL_CALL_TEMPFOO()\n\t%s\nend\nGLOBAL_CALL_TEMPFOO()\n", sn);
	maxsize = strlen(outbuff);
	//memmove(outbuff, sn, maxsize + 1);

	v = luaL_loadbuffer(l, (char*)outbuff, maxsize, "BUFFER");
	if(v != 0)
	{
		LogError("cant load buffer, error %s\nBuffer:\n%s\n", lua_tostring(l, -1), outbuff);
		rc = -1;
	}
	else
	{
		v = lua_pcall(l, 0, 0, 0);
		if(v != 0)
		{
			LogError("lua error : %s\n", lua_tostring(l, -1));
			rc = -1;
		}
	}

	free(outbuff);
	lua_gc(l, LUA_GCCOLLECT, 0);

	return rc;
}

/* register lua funcs */
void RegisterViewerFuncs(uViewFile *gb, lua_State *l)
{
	luaL_openlibs(l);

	//RegisterCommonFuncs(gb->gd, l);
	//RegisterGlobalData(gb->gd, l);
	RegisterLuaFuncs(gb->gd, l);
	RegisterViewerData(gb, l);

/* LUA EXT REF */
/* LUA END EXT REF */

	lua_settop(l, 0);
}

