#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

enum
{
	STYLE_TITLE = 1,
	STYLE_NORMAL,
	STYLE_HIGHLIGHT,

	CLR_BLACK,
	CLR_RED,
	CLR_GREEN,
	CLR_BROWN,
	CLR_BLUE,
	CLR_MAGENTA,
	CLR_CYAN,
	CLR_GREY,
	CLR_DK_GREY,
	CLR_BR_RED,
	CLR_BR_GREEN,
	CLR_YELLOW,
	CLR_BR_BLUE,
	CLR_BR_MAGENTA,
	CLR_BR_CYAN,
	CLR_WHITE
};

typedef struct udtGlobals uGlobalData;


typedef struct udtDirEntry
{
	char *name;
	struct stat stat_buff;
} uDirEntry;

typedef struct udtWindow
{
	int	width;
	int height;

	int offset_row;
	int offset_col;

	// stuff to carry for the file list
	int	highlight_line;
	int top_line;
} uWindow;

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

	void (*draw_border)(uWindow *win);
} uScreenDriver;

#define WINDOW_LEFT	0
#define WINDOW_RIGHT 1

struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

	char	*left_dir;
	char	*right_dir;

	char	*startup_path;

	DList	*lstLeft;
	DList	*lstRight;

	int		selected_window;

	uScreenDriver *screen;

	uint32_t	clr_hi_background;
	uint32_t	clr_hi_foreground;

	uint32_t	clr_background;
	uint32_t	clr_foreground;

	uint32_t	clr_title_fg;
	uint32_t	clr_title_bg;

	uWindow		*win_left;
	uWindow		*win_right;


	// user info
	char	*strRealName;
	char	*strLoginName;
	char	*strHomeDirectory;
	char	*strShell;
	uint32_t	uid;
	uint32_t	gid;

};


extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H

