#ifndef PORTABILITY_H
#define PORTABILITY_H
#ifdef __cplusplus
extern "C"{
#endif

extern char* ALFC_getenv(const char *s);
extern void ALFC_GetUserInfo(uGlobalData *gd);
extern uint64_t ALFC_GetFileSize(uDirEntry *de);
extern uint32_t ALFC_GetFileAttrs(uDirEntry *de);
extern time_t ALFC_GetFileTime(uDirEntry *de);
extern int ALFC_stat(char *fn, struct stat *buff);
extern int ALFC_mkdir(char *s);
extern int ALFC_rmdir(char *s);
extern int ALFC_unlink(char *s);
extern char* ALFC_get_last_error(int err);
extern int ALFC_IsHidden(char *fn, uint32_t attrs);
extern int ALFC_IsExec(char *fn, uint32_t attrs);
extern int ALFC_IsDir(uint32_t attrs);
extern char* ALFC_get_basepath(void);

extern int ALFC_startup(void);
extern int ALFC_shutdown(void);

extern const char *ALFC_str_pathsep;
extern const char ALFC_pathsep;
extern const char ALFC_path_varset;

#ifdef __cplusplus
};
#endif
#endif
