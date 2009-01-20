#ifndef PORTABILITY_H
#define PORTABILITY_H
#ifdef __cplusplus
extern "C"{
#endif

extern char* ALFC_getenv(const char *s);
extern void ALFC_GetUserInfo(uGlobalData *gd);
extern uint64_t ALFC_GetFileSize(uDirEntry *de, struct stat *buff);
extern uint32_t ALFC_GetFileAttrs(uDirEntry *de, struct stat *buff);
extern time_t ALFC_GetFileTime(uDirEntry *de, struct stat *buff);

#ifdef __cplusplus
};
#endif
#endif
