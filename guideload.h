#ifndef GUIDELOAD_H_
#define GUIDELOAD_H_
#ifdef __cplusplus
extern "C"{
#endif

extern void FreePage(uIM_GuidePage *page);
extern void FreeGuide(uIM_GuideHeader *g);
extern uIM_GuideHeader* LoadGuide(char *fn);
extern uIM_GuidePage* ReflowPage(uIM_GuideHeader *hdr, int depth, char **titles);

#ifdef __cplusplus
}
#endif
#endif /* GUIDELOAD_H_ */
