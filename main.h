#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif


typedef struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

} uGlobalData;

extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H