#include "headers.h"


/* build with lua_helper.lua in tools */

lua_State *_Gl;

void ExecuteScript(char *sn)
{
	char *outbuff;
	int maxsize;

	maxsize = 0;
	outbuff = NULL;

	lua_State *l;
	int v;

	// read file into outbuff + maxsize

	l = lua_open();
	RegisterGameFuncs(l);

	v = luaL_loadbuffer(l, (char*)outbuff, maxsize, sn);
	if(v != 0)
	{
		LogWrite("cant load file : %s, error %s", sn, lua_tostring(l, -1));

		lua_close(l);

	}
	else
	{
		v = lua_pcall(l, 0, 0, 0);
		if(v != 0)
		{
			LogWrite("lua error : %s", lua_tostring(l, -1));
		}
	}

	lua_close(l);
	free(outbuff);
}

/* register lua funcs */
void RegisterGameFuncs(lua_State *l)
{
	luaopen_base(l);
	luaopen_string(l);
	luaopen_table(l);

/* LUA EXT REF */
/* LUA END EXT REF */

	lua_pushnumber(l, VMAJ); lua_setglobal(l, "VERSION_MAJ");
	lua_pushnumber(l, VMIN); lua_setglobal(l, "VERSION_MIN");
	lua_pushnumber(l, VBUILD); lua_setglobal(l, "VERSION_BUILD");

	lua_settop(l, 0);
}

