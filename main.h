#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif


typedef struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

	char	*left_dir;
	char	*right_dir;

	char	*startup_path;

} uGlobalData;

extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H