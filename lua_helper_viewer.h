#ifndef LUA_HELPER_VIEWER_H
#define LUA_HELPER_VIEWER_H
#ifdef __cplusplus
extern "C"{
#endif

extern int ExecuteGlobalViewerString(uViewFile *gb, char *sn);
extern void RegisterViewerFuncs(uViewFile *gb, lua_State *l);

#ifdef __cplusplus
};
#endif
#endif
