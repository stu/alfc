#ifndef LUA_HELPER_H
#define LUA_HELPER_H
#ifdef __cplusplus
extern "C"{
#endif

extern void ExecuteScript(char *sn);
extern void RegisterGameFuncs(lua_State *l);

#ifdef __cplusplus
};
#endif
#endif
