#ifndef LUA_API_H
#define LUA_API_H
#ifdef __cplusplus
extern "C"{
#endif

extern uGlobalData *pushGlobalData(lua_State *L);
extern uGlobalData *checkGlobalData(lua_State *L, int index);
extern int RegisterGlobalData(uGlobalData *gb, lua_State *l);

/* LUA EXT REF */
extern int gme_debug_msg(lua_State *L);
extern int gme_GetVersionString(lua_State *L);
extern int gme_ConvertDirectoryName(lua_State *L);
extern int gme_ExecuteScript(lua_State *L);
extern int gme_GetOption(lua_State *L);
/* LUA END EXT REF */

#ifdef __cplusplus
};
#endif
#endif

