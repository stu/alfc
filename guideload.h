#ifndef GUIDELOAD_H_
#define GUIDELOAD_H_
#ifdef __cplusplus
extern "C"{
#endif

extern uHelpFile* LoadHelpFile(char *fn);
extern void FreeHelpFile(uHelpFile *hlp);
extern uHelpPage* HelpReflowPage(uHelpFile *hlp, char *section, int width, int link);
extern void FreeHelpPage(uHelpPage *p);
extern void FreeBreadCrumb(void *data);

#ifdef __cplusplus
}
#endif
#endif /* GUIDELOAD_H_ */
