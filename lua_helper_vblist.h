#ifndef LUA_HELPER_VBLIST_H
#define LUA_HELPER_VBLIST_H
#ifdef __cplusplus
extern "C"{
#endif

extern int ExecuteGlobalListString(uListFile *gb, char *sn);
extern void RegisterListFuncs(uListFile *gb, lua_State *l);

#ifdef __cplusplus
};
#endif
#endif
