#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

enum
{
	ALFC_KEY_DOWN = 0x1000,
	ALFC_KEY_UP,
	ALFC_KEY_LEFT,
	ALFC_KEY_RIGHT,

	ALFC_KEY_INS,
	ALFC_KEY_DEL,
	ALFC_KEY_BACKSPACE,
	ALFC_KEY_HOME,
	ALFC_KEY_END,
	ALFC_KEY_PAGE_UP,
	ALFC_KEY_PAGE_DOWN,
	ALFC_KEY_ENTER,
	ALFC_KEY_TAB,
	ALFC_KEY_SPACE,

	ALFC_KEY_SLEFT,
	ALFC_KEY_SRIGHT,

	ALFC_KEY_F00,
	ALFC_KEY_F01,
	ALFC_KEY_F02,
	ALFC_KEY_F03,
	ALFC_KEY_F04,
	ALFC_KEY_F05,
	ALFC_KEY_F06,
	ALFC_KEY_F07,
	ALFC_KEY_F08,
	ALFC_KEY_F09,
	ALFC_KEY_F10,
	ALFC_KEY_F11,
	ALFC_KEY_F12,

	ALFC_KEY_ALT = 0x1400,
	ALFC_KEY_CTRL = 0x1600,


};

enum
{
	STYLE_TITLE = 1,
	STYLE_NORMAL,
	STYLE_HIGHLIGHT,

	STYLE_EDIT_EOL,

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

#define MAX_STYLES	4

typedef struct udtGlobals uGlobalData;
typedef struct udtScreenDriver uScreenDriver;


enum eFileOpType
{
	eOp_Delete = 1,
	eOp_Copy,
	eOp_Move
};

typedef struct udtFileOperation
{
	int		type;

	int		result_code;
	char	*result_msg;

	union
	{
		struct
		{
			char	*source_filename;
			char	*source_path;
		} udtDelete;

		struct
		{
			char *source_filename;
			char *source_path;

			char *dest_filename;
			char *dest_path;
		} udtMove;

		struct
		{
			char *source_filename;
			char *source_path;

			char *dest_filename;
			char *dest_path;
		} udtCopy;
	} op;
} uFileOperation;


enum eMode
{
	eMode_Directory = 1,
	eMode_Viewer,
};

typedef struct udtKeyBinding
{
	uint32_t	key;
	char		*sCommand;
	char		*sTitle;
} uKeyBinding;

typedef struct udtDirEntry
{
	uint8_t	tagged;

	char 		*name;
	uint64_t	size;
	uint32_t	attrs;
	time_t		time;
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

	int		tagged_count;
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
	void (*print_abs)(const char *s);

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

	char	*optfilehistory;

	char	*left_dir;
	char	*right_dir;

	char	*startup_path;

	DList	*lstLeft;
	DList	*lstRight;

	DList	*lstFullLeft;
	DList	*lstFullRight;

	DList	*lstFilterLeft;
	DList	*lstFilterRight;

	DList	*lstGlobLeft;
	DList	*lstGlobRight;

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

	int			hist_idx;
	DList		*lstLogHistory;

	DList		*lstMRULeft;
	DList		*lstMRURight;

	// global lua funcs
	char 		*gcode;
	lua_State	*_GL;

	DList		*lstHotKeys;

	int			date_fmt_len;
	char		date_fmt[16];

	int			time_fmt_len;
	char		time_fmt[16];

	char		columns[16];

	int			mode;

	DList		*lstFileOps;
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

enum eTimeSigils
{
	et_Hour12 = 1,
	et_Hour24,
	et_Min,
	et_Sec,
	et_AMPM,

	et_Year4,
	et_Year2,
	et_Month,
	et_Day,
	et_MonthNameFull,
	et_MonthNameShort,
};

extern void AddHistory(uGlobalData *gd, char *str, ...);

extern int IsVisible(uGlobalData *gd, int idx);

extern void tag(uGlobalData *gd);
extern int scroll_up(uGlobalData *gd);
extern int scroll_down(uGlobalData *gd);
extern int scroll_home(uGlobalData *gd);
extern int scroll_end(uGlobalData *gd);
extern void scroll_page_down(uGlobalData *gd);
extern void scroll_page_up(uGlobalData *gd);
extern int godir(uGlobalData *gd, char *dir);
extern int updir(uGlobalData *gd);
extern int downdir(uGlobalData *gd);
extern int change_dir(uGlobalData *gd, char *dir);

extern DList* GetActGlob(uGlobalData *gd);
extern DList* GetActFullList(uGlobalData *gd);
extern DList* GetActFilter(uGlobalData *gd);
extern DList* GetActiveMRU(uGlobalData *gd);
extern char* GetActDPath(uGlobalData *gd);
extern DList* GetActList(uGlobalData *gd);
extern uWindow* GetActWindow(uGlobalData *gd);
extern char* GetInActDPath(uGlobalData *gd);
extern DList* GetInActList(uGlobalData *gd);
extern uWindow* GetInActWindow(uGlobalData *gd);
extern uDirEntry* GetHighlightedFile(DList *lstFiles, int idx, int tr);

extern int GetFileIndex(DList *lstFiles, char *name);
extern int SetHighlightedFile(uGlobalData *gd, int idx);
extern uDirEntry* GetFileByName(DList *lstFiles, char *name);
extern void SetQuitAppFlag(int flag);
extern void SwitchPanes(uGlobalData *gd);
extern void SetActivePane(uGlobalData *gd, int p);

extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);
extern void UpdateFilterList(uGlobalData *gd, DList *lstFilter, DList *lstGlob, DList *lstFull, DList *lstF);
extern DList* ResetFilteredFileList(DList *lstF, DList *lstA);

extern void DrawStatusInfoLine(uGlobalData *gd);
extern void DrawFileListWindow(uWindow *win, DList *lstFiles, char *dpath);
extern void DrawActive(uGlobalData *gd);
extern void DrawFilter(uGlobalData *gd);
extern void DrawAll(uGlobalData *gd);

extern void FreeFileOp(void *x);
extern void FreeKey(void *x);
extern uKeyBinding* ScanKey(DList *lst, int key);
extern char* ConvertKeyToName(int key);
extern void DrawMenuLine(uScreenDriver *screen, DList *lstHotKeys);
extern uint32_t fletcher32(uint16_t *data, size_t len);

extern void UpdateDir(uGlobalData *gd, char *set_to_highlight);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H

