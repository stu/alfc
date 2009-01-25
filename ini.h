#ifndef _INIFILE
#define _INIFILE

#ifdef __cplusplus
extern "C"{
#endif

typedef struct INIFILE
{
	DList *lstGroups;
} INIFILE;

extern INIFILE *INI_load(char *fname);
extern void INI_unload(INIFILE *f);
extern char* INI_get(INIFILE *ini, char *group, char *item);

extern INIFILE* INI_EmptyINF(void);
extern int INI_save(char *fname, INIFILE *f);
extern void INI_UpdateItem(INIFILE *f, char *group, char *key, char *val);

#ifdef __cplusplus
};
#endif
#endif
