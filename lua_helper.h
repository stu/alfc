#ifndef LUA_HELPER_H
#define LUA_HELPER_H
#ifdef __cplusplus
extern "C"{
#endif

struct lstr
{
	size_t length;
	char *data;
};

#define GET_LUA_STRING(a,b) (a.data = (char*)luaL_checklstring(L, (b), &a.length))


extern int ExecuteScript(char *sn);
extern void RegisterGameFuncs(lua_State *l);

#ifdef __cplusplus
};
#endif
#endif
