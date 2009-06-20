/****h* ALFC/Lua_Helper
 * FUNCTION
 *   Helper functions for dealing with Lua scripts
 *****
 */

#include "headers.h"

static char* reparse(char *in)
{
	char *x = malloc(128 + (strlen(in)+1) * 2);
	char *p = in;
	char *q = x;

	strcpy(x, "local __xtemp = function()\n");
	q = strchr(x, 0x0);

	while(*p != 0)
	{
		switch(*p)
		{
			case '%':
				//*q ++ = '\\';
				*q ++ = *p;
				break;

			case '\"':
				//*q ++ = *p;
				*q ++ = *p;
				break;

			default:
				*q++ = *p;
				break;

		}

		*q = 0;
		p++;
	}

	strcat(x, "\nend\n__xtemp()\n");

	return x;
}

/* build with lua_helper.lua in tools */
int ExecuteString(uGlobalData *gb, char *sn)
{
	char *outbuff;
	int maxsize;
	int rc = 0;

	lua_State *l;
	int v;

	char *new_sn = reparse(sn);

	maxsize = strlen(new_sn);
	outbuff = malloc(64 + maxsize);

	if(outbuff == NULL)
	{
		LogError("No memory for buffer\n");
		return -1;
	}

	memmove(outbuff, new_sn, maxsize + 1);

	l = lua_open();
	RegisterLuaFuncs(gb, l);

	//LogInfo("exec : %s\n", outbuff);
	v = luaL_loadbuffer(l, (char*)outbuff, maxsize, "BUFFER");
	if(v != 0)
	{
		LogError("cant load buffer, error %s\nBuffer = [%s]\n", lua_tostring(l, -1), new_sn);
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

	lua_close(l);
	free(outbuff);

	return rc;
}

int ExecuteScript(uGlobalData *gb, char *sn)
{
	char *outbuff;
	int maxsize;
	FILE *fp;
	char *fnx;

	int rc = 0;

	lua_State *l;
	int v;

	fnx = ConvertDirectoryName(sn);
	fp = fopen(fnx, "rb");
	if(fp == NULL)
	{
		LogError("cant load file : %s\n", sn);
		free(fnx);
		return -1;
	}

	fseek(fp, 0x0L, SEEK_END);
	maxsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	outbuff = malloc(64 + maxsize);
	if(outbuff == NULL)
	{
		LogError("No memory for script %s\n", sn);
		fclose(fp);
		return -1;
	}

	fread(outbuff, 1, maxsize, fp);
	fclose(fp);

	l = lua_open();
	RegisterLuaFuncs(gb, l);

	v = luaL_loadbuffer(l, (char*)outbuff, maxsize, sn);
	if(v != 0)
	{
		LogError("cant load file : %s, error %s\n", sn, lua_tostring(l, -1));
		rc = -1;
	}
	else
	{
		v = lua_pcall(l, 0, 0, 0);
		if(v != 0)
		{
			LogError("script : %s\n", sn);
			LogError("lua error : %s\n", lua_tostring(l, -1));
			rc = -1;
		}
	}

	lua_close(l);
	free(outbuff);
	free(fnx);

	return rc;
}


int LoadGlobalScript(uGlobalData *gd, char *sn)
{
	uint32_t maxsize;
	FILE *fp;
	int v;
	char *cpath;

	if(sn == NULL)
	{
		LogError("LoadGlobalScript : Cant find global script file. Please setup your options.\n");
		return -1;
	}

	cpath = ConvertDirectoryName(sn);

	fp = fopen(cpath, "rb");
	if(fp == NULL)
	{
		LogError("LoadGlobalScript : Cant load file : %s\n", cpath);
		free(cpath);
		return -1;
	}

	fseek(fp, 0x0L, SEEK_END);
	maxsize = ftell(fp);
	fseek(fp, 0x0L, SEEK_SET);

	gd->gcode = malloc(64 + maxsize);
	if(gd->gcode == NULL)
	{
		LogError("LoadGlobalScript : No memory for script %s\n", cpath);
		fclose(fp);
		free(cpath);
		return -1;
	}

	fread(gd->gcode, 1, maxsize, fp);
	fclose(fp);
	free(cpath);

	gd->_GL = lua_open();
	RegisterLuaFuncs(gd, gd->_GL);

	luaL_loadbuffer(gd->_GL, gd->gcode, maxsize, "GlobalLuaFuncs");

	// call to register globals
	v = lua_pcall(gd->_GL, 0, 0, 0);			/* call 'SetGlobals' with 0 arguments and 0 result */
	if(v != 0)
	{
		LogError("LoadGlobalScript : _gl lua error : %s\n", lua_tostring(gd->_GL, -1));
		return 1;
	}

	return 0;
}

int ExecuteGlobalString(uGlobalData *gb, char *sn)
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


/****f* Lua_Helper/CallGlobalFunc
* FUNCTION
*	This does the acual call, setup must be done via CallGlobalFunc_Setup
* SYNOPSIS
*/
int CallGlobalFunc(lua_State *GL, char *funcname, char *params, ...)
/*
* INPUTS
*	o uGlobalData -- standard for all functions
*	o funcname -- Name of function to call
*	o params -- String list of parametes (s=string, i=int)
*	o ... -- parameter list.. its much like printf
* RETURNS
*   error:
*	o n -- Number of arguements returned from script
*	o -1 -- Failed (logged as type LOG_ERROR)
* AUTHOR
*	Stu George
* EXAMPLE
ret_count = CallGlobalFunc(gd->_GL, "MyFunc", "si,s", "StringParam1", 0x1234, &string_result);
* NOTES
* 	Strings returned from this function must be free'd.
* SOURCE
*/
{
	int v;
	int arg_count;
	int arg_out_count;
	va_list args;
	char *p;
	int flag = 0;

	if(GL == NULL)
		return -1;

	va_start(args, params);
	lua_getfield(GL, LUA_GLOBALSINDEX, funcname);	/* function to be called */

	p = params;
	arg_count = 0;

	while(*p != 0x0 && flag == 0)
	{
		switch(*p++)
		{
			case 's':
				lua_pushstring(GL, va_arg(args, char*));
				arg_count += 1;
				break;

			case 'i':
				lua_pushnumber(GL, va_arg(args, int));
				arg_count += 1;
				break;

			default:
				flag = 1;
				break;
		}
	}

	arg_out_count = strlen(p);

	v = lua_pcall(GL, arg_count, arg_out_count, 0);
	if(v != 0)
	{
		LogError("global lua error : %s\n", lua_tostring(GL, -1));
		va_end(args);
		return -1;
	}


	v = 0;
	arg_out_count = 0 - arg_out_count;

	while(*p != 0x0)
	{
		switch(*p++)
		{
			case 'i':
				if(!lua_isnumber(GL, arg_out_count))
					LogError("Expected integer.. but was not!");
				else
					*va_arg(args, int*) = lua_tointeger(GL, arg_out_count);
				break;

			case 's':
				if(!lua_isstring(GL, arg_out_count))
					LogError("Expected string.. but was not!");
				else
				{
					const char *x;
					x = lua_tostring(GL, arg_out_count);
					if(x != NULL)
						*va_arg(args, char**) = (char*)strdup(x);
					else
						*va_arg(args, char**) = NULL;
				}
				break;

			default:
				break;
		}

		v += 1;
		arg_out_count += 1;
	}

	va_end(args);
	lua_gc(GL, LUA_GCCOLLECT, 0);

	return v;
}
/******
*
*/

/* register lua funcs */
void RegisterLuaFuncs(uGlobalData *gb, lua_State *l)
{
	luaL_openlibs(l);

	RegisterCommonFuncs(gb, l);
	RegisterGlobalData(gb, l);

/* LUA EXT REF */
/* LUA END EXT REF */

	lua_settop(l, 0);

}

