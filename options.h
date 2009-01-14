#ifndef _OPTIONS_H
#define _OPTIONS_H
#ifdef __cplusplus
extern "C"{
#endif

extern int decode_colour(char *s, int def);
extern void CreateHomeDirectory(void);
extern int IsTrue(const char *s);
extern void LoadOptions(uGlobalData *gdata);
extern void SaveOptions(uGlobalData *gdata);

#ifdef __cplusplus
}
#endif
#endif // _OPTIONS_H
