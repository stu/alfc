#ifndef GUIDELOAD_H_
#define GUIDELOAD_H_
#ifdef __cplusplus
extern "C"{
#endif

extern uHelpFile* LoadHelpFile(char *fn);
extern void FreeHelpFile(uHelpFile *hlp);

#ifdef ALFC_DATA_STRUCTURES
extern void help_help(uHelpFile *hdr, uWindow *w, char *page, void(*BuildWindowLayout)(uGlobalData*gd));
#endif

#ifdef __cplusplus
}
#endif
#endif /* GUIDELOAD_H_ */
