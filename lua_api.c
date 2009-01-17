/****h* ALFC/LuaAPI
 * FUNCTION
 *   The C<>Lua interface functions used in the Lua scripts.
 *****
 */

#include "headers.h"

#define GLOBALDATA "uGlobalData"
static const char *uGlobalData_Key = "uGlobalData";
static uGlobalData* GetGlobalData(lua_State *L)
{
	uGlobalData *gb;

	lua_pushlightuserdata(L, (void *)&uGlobalData_Key);
	lua_gettable(L, LUA_REGISTRYINDEX);
	gb = (uGlobalData*)lua_touserdata(L, -1);

	return gb;
}

int RegisterGlobalData(uGlobalData *gb, lua_State *l)
{
	lua_pushlightuserdata(l, (void*)&uGlobalData_Key);  /* push address */
	lua_pushlightuserdata(l, gb);
	lua_settable(l, LUA_REGISTRYINDEX);

	return 1;
}

static char* rtrim(const char *s)
{
	char *p;
	char *x;

	if(*s == 0x0)
		return strdup("");

	x = strdup(s);

	p = strchr(x, 0x0);
	p -= 1;

	while(p > x && isspace(*p) != 0 )
		p--;

	*(p+1)=0;

	p = strdup(x);
	free(x);

	return p;
}

static char* ltrim(const char *s)
{
	const char *p;

	p = s;
	while(*p != 0x0 && isspace(*p) != 0)
		p++;

	return strdup(p);
}

int gme_trim(lua_State *L)
{
	struct lstr strx;
	char *a, *b;

	GET_LUA_STRING(strx, 1);

	a = ltrim(strx.data);
	b = rtrim(a);

	free(a);
	lua_pushstring(L, b);
	free(b);

	return 1;
}

int gme_ltrim(lua_State *L)
{
	struct lstr strx;
	char *a;

	GET_LUA_STRING(strx, 1);

	a = ltrim(strx.data);
	lua_pushstring(L, a);
	free(a);
	return 1;
}

int gme_rtrim(lua_State *L)
{
	struct lstr strx;
	char *a;

	GET_LUA_STRING(strx, 1);

	a = rtrim(strx.data);
	lua_pushstring(L, a);
	free(a);
	return 1;
}

/****f* LuaAPI/debug_msg
* FUNCTION
*	This outputs a string to the logging window
* SYNOPSIS
debug_msg(message)
* INPUTS
*	o Message (string) -- String to log
* RESULTS
*	o None
* EXAMPLE
debug_msg("Log this string to the logging window")
* AUTHOR
*	Stu George
******
*/
int gme_debug_msg(lua_State *L)
{
	struct lstr msg;

	GET_LUA_STRING(msg, 1);

	LogInfo("%s\n", msg.data);

	return 0;
}


/****f* LuaAPI/GetVersionString
* FUNCTION
*	This function will return a string representation
*	of the program version in the format of %i.%02i/%04i of Major Version,
*	Minor Version and Build Number.
* SYNOPSIS
vers = GetVersionString()
* INPUTS
*	o None
* RESULTS
*	o version (string) -- version number (eg: 1.02.1234)
* EXAMPLE
debug_msg("ALFC version " .. GetVersionString())
* AUTHOR
*	Stu George
******
*/
int gme_GetVersionString(lua_State *L)
{
	char *str = malloc(64);

	sprintf(str, "%i.%02i/%04i", VersionMajor(), VersionMinor(), VersionBuild());
	lua_pushstring(L, str);
	free(str);

	return 1;
}

/****f* LuaAPI/ConvertDirectoryName
* FUNCTION
*	This function will take a path and convert
*	environment strings into an absolute path name
*
* 	Will translate any Environment variable that begins with '$' and is
* 	delimited by '/' (or end of string) and will also substitute '~' for '$HOME'.
*
* SYNOPSIS
newPath = ConvertDirectoryName(Path)
* INPUTS
*	o Path -- The path to convert
* RESULTS
*	o newPath (string) -- Converted path
* EXAMPLE
debug_msg("$HOME == " .. ConvertDirectoryName("$HOME"))
* PORTABILITY
*	On windows machines if "$HOME" is present, it will be used,
*	but if it is not, it will substitute "$USERPROFILE"
* AUTHOR
*	Stu George
******
*/
int gme_ConvertDirectoryName(lua_State *L)
{
	char *dir;
	struct lstr d;

	GET_LUA_STRING(d, 1);

	dir = ConvertDirectoryName(d.data);

	lua_pushstring(L, dir);
	free(dir);

	return 1;
}


/****f* LuaAPI/ExecuteScript
* FUNCTION
*	This function will take a path and convert
*	environment strings into an absolute path name
*
* 	Will translate any Environment variable that begins with '$' and is
* 	delimited by '/' (or end of string) and will also substitute '~' for '$HOME'.
*
* SYNOPSIS
ExecuteScript(ScriptName)
* INPUTS
*	o ScriptName -- The filename of the script to execute
* RESULTS
*	o None
* EXAMPLE
ExecuteScript("$HOME/.alfc/test.lua")
* AUTHOR
*	Stu George
******
*/
int gme_ExecuteScript(lua_State *L)
{
	struct lstr scr;
	char *fn;

	GET_LUA_STRING(scr, 1);

	fn = ConvertDirectoryName(scr.data);
	lua_pushnumber(L, ExecuteScript(GetGlobalData(L), fn));
	free(fn);

	return 1;
}

/****f* LuaAPI/GetOption
* FUNCTION
*	Looks up your program configuration.
*
* SYNOPSIS
opt = GetOption(GroupName, ItemName)
* INPUTS
*	o GroupName (string) -- The top level group the option is in
* 	o ItemName (string) -- The item in question
* RESULTS
*	o opt (string) - Option that had been asked for, or if no
*	  option existed, an empty string is returned.
* EXAMPLE
opt = GetOption("general", "left_startup_directory")
* NOTES
* 	May not match what is on disk, as these options are returned
* 	from the running program, and things are bound to change
* 	as the program runs.
*
*	Options are stored like such;
*
* 		[GroupName]
* 		item = value
* SEE ALSO
* 	SetOption, SaveOptions
* AUTHOR
*	Stu George
******
*/
int gme_GetOption(lua_State *L)
{
	uGlobalData *gd;
	struct lstr group;
	struct lstr item;

	char *q;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(group, 1);
	GET_LUA_STRING(item, 2);

	q = INI_get(gd->optfile, group.data, item.data);
	if(q == NULL)
		lua_pushstring(L, "");
	else
		lua_pushstring(L, q);

	return 1;
}

/****f* LuaAPI/SetOption
* FUNCTION
*	Looks up your program configuration.
*
* SYNOPSIS
SetOption(GroupName, ItemName, Value)
* INPUTS
*	o GroupName (string) -- The top level group the option is in
* 	o ItemName (string) -- The item in question
* 	o Value (string) -- value to store
* RESULTS
*	o None
* EXAMPLE
SetOption("general", "left_startup_directory", "$HOME")
* NOTES
*	Options are stored like such;
*
* 		[GroupName]
* 		item = value
* SEE ALSO
* 	GetOption, SaveOptions
* AUTHOR
*	Stu George
******
*/
int gme_SetOption(lua_State *L)
{
	uGlobalData *gd;
	struct lstr group;
	struct lstr item;
	struct lstr val;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(group, 1);
	GET_LUA_STRING(item, 2);
	GET_LUA_STRING(val, 3);

	INI_UpdateItem(gd->optfile, group.data, item.data, val.data);

	return 0;
}

/****f* LuaAPI/SaveOptions
* FUNCTION
*	Looks up your program configuration.
*
* SYNOPSIS
SaveOptions(FileName)
* INPUTS
*	o FileName (string) -- File to write the inmemory options to
* RESULTS
*	o None
* OUTPUTS
* 	o FileName will be written to disk will all current in-memory options saved
* EXAMPLE
SaveOptions("$HOME/temp/test.ini")
* NOTES
*	Options are stored like such;
*
* 		[GroupName]
* 		item = value
* SEE ALSO
* 	GetOption, SetOption
* AUTHOR
*	Stu George
******
*/
int gme_SaveOptions(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	CreateHomeDirectory();
	INI_save(gd->optfilename, gd->optfile);

	return 0;
}

/****f* LuaAPI/SetCurrentWorkingDirectory
* FUNCTION
*	Change the active window pane to a new directory
* SYNOPSIS
error = SetCurrentWorkingDirectory(dir)
* INPUTS
*	o dir (string) - Path to change into
* RESULTS
*   error (integer) :
*   o 0 -- No Error
* 	o -1 -- Could not scroll down anymore
* EXAMPLE
error = SetCurrentWorkingDirectory("C:/WinNT/system32")
* SEE ALSO
* 	ChangeDirUp, ChangeDirDown, GetCurrentWorkingDirectory
* AUTHOR
*	Stu George
******
*/
int gme_SetCurrentWorkingDirectory(lua_State *L)
{
	struct lstr cdx;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(cdx, 1);

	lua_pushnumber(L, change_dir(gd, cdx.data));

	return 1;
}

/****f* LuaAPI/GetCurrentWorkingDirectory
* FUNCTION
*	Returns the full path of the current active window
* SYNOPSIS
dir = GetCurrentWorkingDirectory()
* INPUTS
*	o None
* RESULTS
*	o dir (string) -- Current full path of the active window
* EXAMPLE
dir = GetCurrentWorkingDirectory()
* SEE ALSO
* 	ChangeDirUp, ChangeDirDown, SetCurrentWorkingDirectory
* AUTHOR
*	Stu George
******
*/
int gme_GetCurrentWorkingDirectory(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	char *x = GetActDPath(gd);
	lua_pushstring(L, x);
	free(x);

	return 1;
}

/****f* LuaAPI/ClearScreen
* FUNCTION
*	Clears the screen and sets the cursor to the upper left
* SYNOPSIS
ClearScreen()
* INPUTS
*	o None
* RESULTS
*	o None
* EXAMPLE
ClearScreen()
* SEE ALSO
* 	GetScreenHeight, GetScreenWidth, Print
* AUTHOR
*	Stu George
******
*/
int gme_ClearScreen(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	gd->screen->cls();

	return 0;
}

/****f* LuaAPI/GetScreenHeight
* FUNCTION
*	Returns the number of rows for the screen height.
* SYNOPSIS
height = GetScreenHeight()
* INPUTS
*	o None
* RESULTS
*	o height (integer) -- Height of the screen in rows
* EXAMPLE
ClearScreen()
* SEE ALSO
* 	ClearScreen, GetScreenWidth, Print
* AUTHOR
*	Stu George
******
*/
int gme_GetScreenHeight(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->screen->get_screen_height());

	return 1;
}

/****f* LuaAPI/GetScreenWidth
* FUNCTION
*	Returns the number of columns for the screen width.
* SYNOPSIS
width = GetScreenWidth()
* INPUTS
*	o None
* RESULTS
*	o width (integer) -- Width of the screen in columns
* EXAMPLE
width = GetScreenWidth()
* SEE ALSO
* 	ClearScreen, GetScreenHeight, Print
* AUTHOR
*	Stu George
******
*/
int gme_GetScreenWidth(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	lua_pushnumber(L, gd->screen->get_screen_width());

	return 1;
}

/****f* LuaAPI/Print
* FUNCTION
*	Puts a string on the screen
* SYNOPSIS
Print(message)
* INPUTS
*	o message -- String to print to the screen
* RESULTS
*	o None
* EXAMPLE
Print("This is v" .. GetVersionString() .. " of ALFC")
* SEE ALSO
* 	ClearScreen, GetScreenHeight, GetScreenWidth
* AUTHOR
*	Stu George
******
*/
int gme_Print(lua_State *L)
{
	struct lstr msg;
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(msg, 1);

	gd->screen->print(msg.data);

	return 0;
}

/****f* LuaAPI/GetUserID
* FUNCTION
*	Gets the User ID from the system
* SYNOPSIS
uid = GetUserID()
* INPUTS
*	o None
* RESULTS
*	o uid (integer) Users ID on the system
* PORTABILITY
* 	On Unix machines returns the UID. On Windows machines it returns a default of INT_MAX
* SEE ALSO
* 	GetUserGroup
* AUTHOR
*	Stu George
******
*/
int gme_GetUserID(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->uid);
	return 1;
}

/****f* LuaAPI/GetUserGroup
* FUNCTION
*	Gets the primary user group id for the person logged into that machine
* SYNOPSIS
gid = GetUserGroup()
* INPUTS
*	o None
* RESULTS
*	o gid (integer) Users primary Group ID on the system
* PORTABILITY
* 	On Unix machines returns the GID. On Windows machines it returns a default of INT_MAX
* SEE ALSO
* 	GetUserID
* AUTHOR
*	Stu George
******
*/
int gme_GetUserGroup(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->gid);
	return 1;
}

int gme_SetColour(lua_State *L)
{
	//SetColour(title_background, "red")
	uGlobalData *gd;
	int style;
	struct lstr clr;
	uint32_t c;

	style = luaL_checknumber(L, 1);
	GET_LUA_STRING(clr, 2);

	gd = GetGlobalData(L);
	assert(gd != NULL);

	c = decode_colour(clr.data, -1);
	if(c == -1)
		return luaL_error(L, "Unknown colour %s", clr.data);

	switch(style)
	{
		case e_highlight_background:
			gd->clr_hi_background = c;
			gd->screen->init_style(STYLE_HIGHLIGHT, gd->clr_hi_foreground, gd->clr_hi_background);
			break;
		case e_highlight_foreground:
			gd->clr_hi_foreground = c;
			gd->screen->init_style(STYLE_HIGHLIGHT, gd->clr_hi_foreground, gd->clr_hi_background);
			break;
		case e_background:
			gd->clr_background = c;
			gd->screen->init_style(STYLE_NORMAL, gd->clr_foreground, gd->clr_background);
			break;
		case e_foreground:
			gd->clr_foreground = c;
			gd->screen->init_style(STYLE_NORMAL, gd->clr_foreground, gd->clr_background);
			break;
		case e_title_foreground:
			gd->clr_title_fg = c;
			gd->screen->init_style(STYLE_TITLE, gd->clr_title_fg, gd->clr_title_bg);
			break;
		case e_title_background:
			gd->clr_title_bg = c;
			gd->screen->init_style(STYLE_TITLE, gd->clr_title_fg, gd->clr_title_bg);
			break;
	}

	return 0;
}

/****f* LuaAPI/GetRuntimeOption_CompressFilesize
* FUNCTION
*	Returns the in-memory option of CompressFilesize
* SYNOPSIS
flag = GetRuntimeOption_CompressFilesize()
* INPUTS
*	o None
* RESULTS
*	flag (integer):
* 	o 0 - Turn off Compressed filesizes
* 	o 1 - Turn on Compressed filesizes
* SEE ALSO
* 	SetRuntimeOption_CompressFilesize
* AUTHOR
*	Stu George
******
*/
int gme_GetRuntimeOption_CompressFilesize(lua_State *L)
{
	uGlobalData *gd;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->compress_filesize);
	return 1;
}

/****f* LuaAPI/SetRuntimeOption_CompressFilesize
* FUNCTION
*	Sets the in-memory option of CompressFilesize without changing the ondisk option.
* SYNOPSIS
SetRuntimeOption_CompressFilesize(Flag)
* INPUTS
*	flag (integer):
* 	o 0 - Turn off Compressed filesizes
* 	o 1 - Turn on Compressed filesizes
* RESULTS
*   o None
* SEE ALSO
* 	GetRuntimeOption_CompressFilesize
* AUTHOR
*	Stu George
******
*/
int gme_SetRuntimeOption_CompressFilesize(lua_State *L)
{
	uGlobalData *gd;
	int v;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = luaL_checknumber(L, 1);
	if(v == 1)
		gd->compress_filesize = 1;
	else
		gd->compress_filesize = 0;

	return 0;
}


/****f* LuaAPI/SetQuitAppFlag
* FUNCTION
*	Sets the flag to tell the application to quit.
* SYNOPSIS
SetQuitAppFlag()
* INPUTS
*	o None
* RESULTS
*   o None
* NOTES
* 	This does not cause the app to quit there and then, it will most likely fiinish whatever
* 	processing it was doing and once it gets back to the main screen, will quit out.
* AUTHOR
*	Stu George
******
*/
int gme_SetQuitAppFlag(lua_State *L)
{
	SetQuitAppFlag(1);
	return 0;
}

/****f* LuaAPI/SwitchPanes
* FUNCTION
*	Just like the user had pressed TAB to toggle between panes.
* SYNOPSIS
activePane = SwitchPanes()
* INPUTS
*	o None
* RESULTS
*   o activePane -- Returns the active window Pane (either WINDOW_LEFT or WINDOW_RIGHT)
* SEE ALSO
* 	SetActivePane, GetActivePane
* AUTHOR
*	Stu George
******
*/
int gme_SwitchPanes(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	SwitchPanes(gd);
	lua_pushnumber(L, gd->selected_window);
	return 1;
}

/****f* LuaAPI/GetActivePane
* FUNCTION
*	Returns which window is active
* SYNOPSIS
activePane = GetActivePane()
* INPUTS
*	o None
* RESULTS
*   o activePane -- Returns the active window Pane (either WINDOW_LEFT or WINDOW_RIGHT)
* SEE ALSO
* 	SetActivePane, SwitchPanes
* AUTHOR
*	Stu George
******
*/
int gme_GetActivePane(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, gd->selected_window);
	return 1;
}

/****f* LuaAPI/SetActivePane
* FUNCTION
*	Sets the active window pane on the screen to either the left or right.
* SYNOPSIS
activePane = SetActivePane(pane)
* INPUTS
*	o pane -- Either WINDOW_LEFT or WINDOW_RIGHT
* RESULTS
*   o activePane -- Returns the active window Pane (either WINDOW_LEFT or WINDOW_RIGHT)
* SEE ALSO
* 	GetActivePane, SwitchPanes
* AUTHOR
*	Stu George
******
*/
int gme_SetActivePane(lua_State *L)
{
	int v;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = luaL_checknumber(L, 1);

	if(v == WINDOW_LEFT || v == WINDOW_RIGHT)
		SetActivePane(gd, v);
	else
		return luaL_error(L, "Unknown window %i (Must be WINDOW_LEFT or WINDOW_RIGHT)", v);

	lua_pushnumber(L, gd->selected_window);
	return 1;
}

/****f* LuaAPI/ScrollDown
* FUNCTION
*	Same as the user pressing the Down Arrow key to scroll downward the active window pane
* SYNOPSIS
error = ScrollDown(LineCount)
* INPUTS
*	o LineCount (integer) -- The number of lines to Scroll down by
* RESULTS
*   error (integer) :
*   o 0 -- No Error
* 	o -1 -- Could not scroll down anymore
* SEE ALSO
*	ScrollHome, ScrollEnd, ScrollUp, ScrollPageDown, ScrollPageUp
* AUTHOR
*	Stu George
******
*/
int gme_ScrollDown(lua_State *L)
{
	int v;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = luaL_checknumber(L, 1);

	while(v > 0)
	{
		v -= 1;
		if(scroll_down(gd) == -1)
		{
			lua_pushnumber(L, -1);
			return 1;
		}
	}

	lua_pushnumber(L, 0);
	return 1;
}

/****f* LuaAPI/ScrollUp
* FUNCTION
*	Same as the user pressing the Down Arrow key to scroll downward the active window pane
* SYNOPSIS
error = ScrollUp(LineCount)
* INPUTS
*	o LineCount (integer) -- The number of lines to Scroll up by
* RESULTS
*   error (integer) :
*   o 0 -- No Error
* 	o -1 -- Could not scroll up anymore
* SEE ALSO
*	ScrollHome, ScrollEnd, ScrollDown, ScrollPageDown, ScrollPageUp
* AUTHOR
*	Stu George
******
*/
int gme_ScrollUp(lua_State *L)
{
	int v;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	v = luaL_checknumber(L, 1);

	while(v > 0)
	{
		v -= 1;
		if(scroll_up(gd) == -1)
		{
			lua_pushnumber(L, -1);
			return 1;
		}
	}

	lua_pushnumber(L, 0);
	return 1;
}

/****f* LuaAPI/ScrollEnd
* FUNCTION
*	Same as pressing 'END' key, will move highlight bar to bottom of the active window pane
* SYNOPSIS
ScrollHome()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
*	ScrollHome, ScrollDown, ScrollUp, ScrollPageDown, ScrollPageUp
* AUTHOR
*	Stu George
******
*/
int gme_ScrollEnd(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	scroll_end(gd);

	return 0;
}

/****f* LuaAPI/ScrollHome
* FUNCTION
*	Same as pressing 'HOME' key, will move highlight bar to top of active window pane
* SYNOPSIS
ScrollHome()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
*	ScrollEnd, ScrollDown, ScrollUp, ScrollPageDown, ScrollPageUp
* AUTHOR
*	Stu George
******
*/
int gme_ScrollHome(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	scroll_home(gd);

	return 0;
}

/****f* LuaAPI/GetHighlightedFilename
* FUNCTION
*	Will return the currently selected item
*	from the active window pane.
* SYNOPSIS
filename = GetHighlightedFilename()
* INPUTS
*	o None
* RESULTS
*	o filename (string) -- Currently highlighted name in active window pane
* SEE ALSO
* 	SetHighlightedFile
* AUTHOR
*	Stu George
******
*/
int gme_GetHighlightedFilename(lua_State *L)
{
	uDirEntry *de;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line - 1, GetActWindow(gd)->top_line);

	lua_pushstring(L, de->name);
	return 1;
}

/****f* LuaAPI/TagHighlightedFile
* FUNCTION
*	Will toggle the tagged flag of the file currently highlighted
* 	in the currently active window.
*
* 	This function will update the screen, the 'Tagged file' info
* 	line will be updated accordingly and the highlight will be moved to
* 	the next line.
*
* 	This function follows the same behaviour as if the user had pressed F12
* 	(default key for toggling the tag on a file).
* SYNOPSIS
TagHighlightedFile()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
*	TagFile, GetTaggedFileCount
* AUTHOR
*	Stu George
******
*/
int gme_TagHighlightedFile(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	tag(gd);

	return 0;
}

/****f* LuaAPI/ChangeDirUp
* FUNCTION
*	Executes a ".." to traverse up one level.
* SYNOPSIS
error = ChangeDirUp()
* INPUTS
*	o None
* RESULTS
*	error (integer):
*	o 0 -- No Error
*	o -1 -- Could not change down into directory
* SEE ALSO
*	ChangeDirDown, GetCurrentWorkingDirectory, SetCurrentWorkingDirectory
* AUTHOR
*	Stu George
******
*/
int gme_ChangeDirUp(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, updir(gd));
	return 1;
}

/****f* LuaAPI/ChangeDirDown
* FUNCTION
*	Will take the currently highlighted file
*	and enter it (if it is i a directory).
* SYNOPSIS
error = ChangeDirDown()
* INPUTS
*	o None - Uses currently highlighted filename in the active window
* RESULTS
*	error (integer):
*	o 0 -- No Error
*	o -1 -- Could not change down into directory
* SEE ALSO
*	ChangeDirUp, GetCurrentWorkingDirectory, SetCurrentWorkingDirectory
* AUTHOR
*	Stu George
******
*/
int gme_ChangeDirDown(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, downdir(gd));
	return 1;
}

/****f* LuaAPI/TagFile
* FUNCTION
*	Will toggle the tagged mark for the given filename in the active window.
*
*	This function will update the screen, The file will be visible
*	marked if it appears in the current view. Second, the 'Tagged file' info
*	line will be updated accordingly.
*
*	This does not alter the current highlighted line or advance the cursor like
*	TagHighlightedFile does
* SYNOPSIS
error = TagFile(strFileName)
* INPUTS
*	o string - Name of the file to be tagged
* RESULTS
*	error (integer):
*	o 0 -- For No Error
*	o -1 -- File not found
* SEE ALSO
*	GetTaggedFileCount, TagHighlightedFile
* AUTHOR
*	Stu George
******
*/
int gme_TagFile(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	struct lstr tf;

	DList *lstFiles;
	DLElement *d;
	uDirEntry *de;

	int count;

	GET_LUA_STRING(tf, 1);

	lstFiles = GetActList(gd);
	d = dlist_head(lstFiles);

	count = 0;
	while(d != NULL)
	{
		de = dlist_data(d);

		if( strcmp(de->name, tf.data) == 0)
		{
			de->tagged ^= 1;
			if(de->tagged == 1)
				GetActWindow(gd)->tagged_count += 1;
			else
				GetActWindow(gd)->tagged_count -= 1;

			// test if its visible
			if( IsVisible(gd, count) == 1)
			{
				count -= GetActWindow(gd)->top_line;

				// redraw filepane
				if(count == GetActWindow(gd)->highlight_line)
					gd->screen->set_style(STYLE_HIGHLIGHT);
				else
					gd->screen->set_style(STYLE_NORMAL);
				gd->screen->set_cursor(2 + GetActWindow(gd)->offset_row + count, GetActWindow(gd)->offset_col + 2 );
				gd->screen->print( de->tagged == 1 ? "+" : " ");
				gd->screen->set_style(STYLE_NORMAL);
			}

			// ?? call from here? or just mark dirty?
			DrawStatusInfoLine(gd);

			lua_pushnumber(L, 0);
			return 1;
		}

		d = dlist_next(d);
		count += 1;
	}

	// no file found by that name
	lua_pushnumber(L, -1);
	return 1;
}

/****f* LuaAPI/GetTaggedFileCount
* FUNCTION
*	Return the count of tagged files from the active window
* SYNOPSIS
count = GetTaggedFileCount()
* INPUTS
*	o None
* RESULTS
*	o integer - Count of tagged files
* SEE ALSO
*	TagFile, TagHighlightedFile
* AUTHOR
*	Stu George
******
*/
int gme_GetTaggedFileCount(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	lua_pushnumber(L, GetActWindow(gd)->tagged_count);
	return 1;
}

/****f* LuaAPI/GetFileList
* FUNCTION
*	Returns a table of file details in the order they are in in memory
* SYNOPSIS
tbl = GetFileList()
* INPUTS
*	o None
* RESULTS
*	o table - table of entries (name, size, directory, tagged)
* SEE ALSO
*	TagFile, TagHighlightedFile
* AUTHOR
*	Stu George
* NOTES
* 	Changing the details in this table does not change whats in memory. This is a local copy only.
******
*/
int gme_GetFileList(lua_State *L)
{
	DLElement *e;
	uDirEntry *de;
	DList *lst;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	int i;

	lst = GetActList(gd);

	lua_newtable(L);

	i = 1;
	e = dlist_head(lst);
	while(e != NULL)
	{
		de = dlist_data(e);

		lua_pushnumber(L, i++);
		lua_newtable(L);

		lua_pushstring(L, "name");
		lua_pushstring(L, de->name);
		lua_settable(L, -3);

		lua_pushstring(L, "size");
		lua_pushnumber(L, de->size);
		lua_settable(L, -3);


		lua_pushstring(L, "directory");
		if( S_ISDIR(de->attrs) == 0)
			lua_pushnumber(L, 0);
		else
			lua_pushnumber(L, 1);
		lua_settable(L, -3);

		lua_pushstring(L, "tagged");
		lua_pushnumber(L, de->tagged);
		lua_settable(L, -3);

		lua_settable(L, -3);

		e = dlist_next(e);
	}

	return 1;
}


/****f* LuaAPI/ScrollPageDown
* FUNCTION
*	Scrolls active window pane down a page. If cursor is not at bottom of the page, it is moved there first
* 	before it will scroll the page
* SYNOPSIS
ScrollPageDown()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
*	ScrollHome, ScrollEnd, ScrollDown, ScrollUp, ScrollPageUp
* AUTHOR
*	Stu George
******
*/
int gme_ScrollPageDown(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	scroll_page_down(gd);

	return 1;
}


/****f* LuaAPI/ScrollPageUp
* FUNCTION
*	Scrolls active window pane up a page. If cursor is not at top of the page, it is moved there first
* 	before it will scroll the page
* SYNOPSIS
ScrollPageUp()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
*	ScrollHome, ScrollEnd, ScrollDown, ScrollUp, ScrollPageDown
* AUTHOR
*	Stu George
******
*/
int gme_ScrollPageUp(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	scroll_page_up(gd);

	return 1;
}

/****f* LuaAPI/SetHighlightedFile
* FUNCTION
*	Will set the highlight bar in the active window. Accepts Key Index or FileName as input.
* SYNOPSIS
index = SetHighlightedFile(FileToHighlight)
* INPUTS
*	o FileToHighlight (integer) -- Key Index of file to highlight
*   o FileToHighlight (string) -- If parameter is string, will search the list for the file to highlight.
* RESULTS
*	o index (integer) -- If failure, returns -1, if no error, returns the index of the file highlighted.
* SEE ALSO
*	GetHighlightedFilename
* AUTHOR
*	Stu George
******
*/
int gme_SetHighlightedFile(lua_State *L)
{
	int idx;
	struct lstr name;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);


	if(lua_isnumber(L, 1) == 1)
	{
		idx = luaL_checknumber(L, 1);
	}
	else
	{
		GET_LUA_STRING(name, 1);
		idx = GetFileIndex(GetActList(gd), name.data);
	}

	if(idx != -1)
	{
		idx = SetHighlightedFile(gd, idx);
		DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
		DrawActive(gd);
	}

	lua_pushnumber(L, idx);

	return 1;
}


