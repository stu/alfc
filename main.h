#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif


#define STYLE_TITLE			1
#define STYLE_NORMAL		2

#define CLR_BLACK			3
#define CLR_RED				4
#define CLR_GREEN			5
#define CLR_BROWN			6
#define CLR_BLUE			7
#define CLR_MAGENTA			8
#define CLR_CYAN			9
#define CLR_GREY			10
#define CLR_DK_GREY			11
#define CLR_BR_RED			12
#define CLR_BR_GREEN		13
#define CLR_YELLOW			14
#define CLR_BR_BLUE			15
#define CLR_BR_MAGENTA		16
#define CLR_BR_CYAN			17
#define CLR_WHITE			18


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

	void (*set_style)(uGlobalData *gdata, int style);
	void (*set_cursor)(int row, int col);
	void (*erase_eol)(void);
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

	uint32_t	clr_background;
	uint32_t	clr_foreground;
	uint32_t	clr_title_fg;
	uint32_t	clr_title_bg;

};


extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H