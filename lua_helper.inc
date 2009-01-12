#include "headers.h"


/* build with lua_helper.lua in tools */

lua_State *_Gl;

void ExecuteScript(char *sn)
{
	char *outbuff;
	int maxsize;
	FILE *fp;

	lua_State *l;
	int v;

	fp = fopen(sn, "r");
	if(fp == NULL)
	{
		LogError("cant load file : %s\n", sn);
		return;
	}

	fseek(fp, 0x0L, SEEK_END);
	maxsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	outbuff = malloc(64 + maxsize);
	if(outbuff == NULL)
	{
		LogError("No memory for script %s\n", sn);
		fclose(fp);
		return;
	}

	fread(outbuff, 1, maxsize, fp);
	fclose(fp);

	l = lua_open();
	RegisterGameFuncs(l);

	v = luaL_loadbuffer(l, (char*)outbuff, maxsize, sn);
	if(v != 0)
	{
		LogError("cant load file : %s, error %s\n", sn, lua_tostring(l, -1));
	}
	else
	{
		v = lua_pcall(l, 0, 0, 0);
		if(v != 0)
		{
			LogError("script : %s\n", sn);
			LogError("lua error : %s\n", lua_tostring(l, -1));
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

