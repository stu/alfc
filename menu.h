#ifndef MENU_H
#define MENU_H
#ifdef __cplusplus
extern "C"{
#endif

//extern void DrawMenu(uGlobalData *gd, int menu_to_open);
extern void DrawMenu(lua_State *L, int menu_to_open);

#ifdef __cplusplus
}
#endif
#endif
