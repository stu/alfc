#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

typedef struct udtGlobals uGlobalData;

typedef struct udtScreenDriver
{
	int (*init)(uGlobalData *gdata);
	int (*deinit)(uGlobalData *gdata);

	void (*cls)(void);

	int (*get_screen_height)(void);
	int (*get_screen_width)(void);

	int (*get_keypress)(void);

	void (*print)(const char *s);
} uScreenDriver;

struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

	char	*left_dir;
	char	*right_dir;

	char	*startup_path;

	DList	*lstLeft;
	DList	*lstRight;

	uScreenDriver *screen;
};


extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H