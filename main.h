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
typedef struct udtScreenDriver uScreenDriver;

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

	uGlobalData *gd;
	uScreenDriver *screen;
} uWindow;

struct udtScreenDriver
{
	uGlobalData *gd;

	int (*init)(uScreenDriver *scr);
	int (*deinit)(void);

	void (*cls)(void);

	int (*get_screen_height)(void);
	int (*get_screen_width)(void);

	uint32_t (*get_keypress)(void);

	void (*print)(const char *s);

	void (*set_style)(int style);
	void (*set_cursor)(int row, int col);
	void (*erase_eol)(void);

	void (*draw_border)(uWindow *win);
	void (*init_style)(int style, uint32_t fg, uint32_t bg);
	void (*print_hline)(void);
	void (*print_vline)(void);
};

#define WINDOW_LEFT	0
#define WINDOW_RIGHT 1
#define MAX_COMMAND_LENGTH	256

struct udtGlobals
{
	char	*optfilename;
	INIFILE *optfile;

	char	*left_dir;
	char	*right_dir;

	char	*startup_path;

	DList	*lstLeft;
	DList	*lstRight;

	int		compress_filesize;
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

	int			command_length;
	char		command[MAX_COMMAND_LENGTH];
};

enum
{
	e_highlight_background = 1,
	e_highlight_foreground,

	e_background,
	e_foreground,

	e_title_foreground,
	e_title_background
};

extern void SetQuitAppFlag(int flag);
extern void SwitchPanes(uGlobalData *gd);
extern void SetActivePane(uGlobalData *gd, int p);

extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H

