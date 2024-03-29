#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C"{
#endif

#define FILETYPE_DEFAULT	0
#define FILETYPE_IMAGE 		1
#define FILETYPE_ARCHIVE 	2
#define FILETYPE_DOC 		3
#define FILETYPE_BACKUP 	4
#define FILETYPE_EXEC       5
#define FILETYPE_MEDIA      6


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
	ALFC_KEY_ESCAPE,
	ALFC_KEY_ESCAPE_ESCAPE,

	ALFC_KEY_SLEFT,
	ALFC_KEY_SRIGHT,

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

	STYLE_HELP_TITLE,
	STYLE_HELP_NORMAL,
	STYLE_HELP_BOLD,
	STYLE_HELP_EMPHASIS,
	STYLE_HELP_LINK,

	STYLE_00,
	STYLE_01,
	STYLE_02,
	STYLE_03,
	STYLE_04,
	STYLE_05,
	STYLE_06,
	STYLE_07,

	STYLE_VIEW_EDIT_EOL = STYLE_00,
	STYLE_VIEW_01,
	STYLE_VIEW_02,
	STYLE_VIEW_03,
	STYLE_VIEW_04,
	STYLE_VIEW_05,
	STYLE_VIEW_06,
	STYLE_VIEW_07,

	STYLE_DIR_EXEC = STYLE_00,
	STYLE_DIR_LINK,
	STYLE_DIR_IMAGE,
	STYLE_DIR_DIR,
	STYLE_DIR_DOCUMENT,
	STYLE_DIR_ARCHIVE,
	STYLE_DIR_BACKUP,
	STYLE_DIR_MEDIA,

	CLR_BLACK = STYLE_07 + 1,
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

#define MAX_STYLES	16

typedef struct udtGlobals uGlobalData;
typedef struct udtScreenDriver uScreenDriver;


typedef struct udtSubMenu
{
	char		*name;
	uint32_t	key;
	char		*code;
} uSubMenu;

typedef struct udtMenu
{
	char*		name;
	uint32_t	key;

	int			count;
	uSubMenu	**child;
} uMenu;


enum eFileOpType
{
	eOp_Delete = 1,
	eOp_Copy,
	eOp_Move,
	eOp_MakeDir,
	eOp_DeleteDir,
	eOp_SymLink
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

			uint64_t source_length;
		} udtDelete;

		struct
		{
			char *source_filename;
			char *source_path;

			char *dest_filename;
			char *dest_path;

			uint64_t source_length;
		} udtMove;

		struct
		{
			char *source_filename;
			char *source_path;

			char *dest_filename;
			char *dest_path;

			uint64_t source_length;
		} udtSymlink;

		struct
		{
			char *source_filename;
			char *source_path;

			char *dest_filename;
			char *dest_path;

			uint64_t source_length;
		} udtCopy;

		struct
		{
			char *source_filename;
			char *source_path;
		} udtMakeDir;

		struct
		{
			char *source_filename;
			char *source_path;
		} udtDeleteDir;

	} op;
} uFileOperation;


enum eMode
{
	eMode_Directory = 1,
	eMode_Viewer,
	eMode_VB_List
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

	char		*path;
	char 		*name;
	uint64_t	size;
	uint32_t	attrs;
	time_t		time;

	char		*lnk;
	uint64_t	lnk_size;

	int			type;
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
	uint64_t tagged_size;

	uint64_t	total_size;
	uint64_t	hidden_count;

} uWindow;

struct udtScreenDriver
{
	uGlobalData *gd;

	char* (*driver_name)(void);

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
	void (*window_clear)(uWindow *win);
	void (*init_style)(int style, uint32_t fg, uint32_t bg);
	void (*print_hline)(void);
	void (*print_vline)(void);
	void (*set_updates)(int set);
	int (*get_updates)(void);

	void (*init_dir_styles)(uScreenDriver *scr);
	void (*init_view_styles)(uScreenDriver *scr);
	void (*init_list_styles)(uScreenDriver *scr);
	int (*screen_isresized)(void);
	int (*screen_isshutdown)(void);
	void (*update_window)(void);
	void (*trigger_redraw)(void);
	void (*going_exec)(void);

	void (*enable_raw)(void);
	void (*disable_raw)(void);
};

#define WINDOW_LEFT	0
#define WINDOW_RIGHT 1
#define MAX_COMMAND_LENGTH	256
#define MAX_MENU 32

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
	DList 		*lstViewerList;

	uMenu		*file_menu[MAX_MENU];
	uMenu		*viewer_menu[MAX_MENU];
	uMenu		*list_menu[MAX_MENU];
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

#define ALFC_DATA_STRUCTURES	1

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
extern char* replace(const char *in, char a, char b);

extern uMenu** GetActMenu(uGlobalData *gd);
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
extern int GetQuitAppFlag(void);
extern void SwitchPanes(uGlobalData *gd);
extern void SetActivePane(uGlobalData *gd, int p);

extern char* GetCurrentWorkingDirectory(void);
extern char* ConvertDirectoryName(const char *x);
extern void UpdateFilterList(uGlobalData *gd, DList *lstFilter, DList *lstGlob, DList *lstFull, DList *lstF);
extern DList* ResetFilteredFileList(uGlobalData *gd, DList *lstF, DList *lstA);

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
extern char* GetDateTimeString(char *fmt, time_t t);
extern void about_window(uGlobalData *gd);

extern void UpdateDir(uGlobalData *gd, char *set_to_highlight);
extern int TagWithGlob(uGlobalData *gd, char *pattern);
extern int TagWithFilter(uGlobalData *gd, char *pattern);
extern DList* GetFiles(char *path);
extern int ALFC_main(int start_mode, char *view_file);
extern void DrawCLI(uGlobalData *gd);

extern void ToggleHidden(uGlobalData *gd);
extern void SetHiddenFlag(uGlobalData *gd, int hidden);
extern int GetHiddenFlag(uGlobalData *gd);
extern void msgbox(uGlobalData *gd, char *str);

extern int ScanMenuOpen(uGlobalData *gd, uint32_t key);
extern void FreeMenuData(uMenu *menu[]);

extern char *start_left;
extern char *start_right;

extern char* gstr_WindowTitle;

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H

