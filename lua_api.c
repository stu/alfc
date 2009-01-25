/****h* ALFC/LuaAPI
 * FUNCTION
 *   The C<>Lua interface functions used in the Lua scripts.
 *****
 */

#include "headers.h"

#define GLOBALDATA "uGlobalData"
static const char *uGlobalData_Key = GLOBALDATA;


uGlobalData* GetGlobalData(lua_State *L)
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

// map function, take from PIL.
int gme_map(lua_State *L)
{
	int i, n;

	/* 1st argument must be a table (t) */
	luaL_checktype(L, 1, LUA_TTABLE);

	/* 2nd argument must be a function (f) */
	luaL_checktype(L, 2, LUA_TFUNCTION);

	n = luaL_getn(L, 1);  /* get size of table */

	for (i=1; i<=n; i++) {
	lua_pushvalue(L, 2);   /* push f */
	lua_rawgeti(L, 1, i);  /* push t[i] */
	lua_call(L, 1, 1);     /* call f(t[i]) */
	lua_rawseti(L, 1, i);  /* t[i] = result */
	}

	return 0;  /* no results */
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

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);

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
	DrawStatusInfoLine(gd);

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

static int ClearGlob(DList *lst)
{
	DLElement *e;
	char *x;

	e = dlist_head(lst);
	while(e != NULL)
	{
		dlist_remove(lst, e, (void*)&x);
		free(x);
		e = dlist_head(lst);
	}

	return 0;
}

static int ClearFilter(DList *lst)
{
	DLElement *e;
	char *x;

	e = dlist_head(lst);
	while(e != NULL)
	{
		dlist_remove(lst, e, (void*)&x);
		free(x);
		e = dlist_head(lst);
	}

	return 0;
}

/****f* LuaAPI/SetFilter
* FUNCTION
*	Removes all existing filters and applies a posix regex filter against the file list.
* SYNOPSIS
SetFilter(filter)
* INPUTS
*	o filter (string) -- Posix regular expression
* RESULTS
*	o None
* SEE ALSO
*	AddFilter, SetGlob, AddGlob
* AUTHOR
*	Stu George
******
*/
int gme_SetFilter(lua_State *L)
{
	struct lstr f;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DLElement *e;
	char *x;

	GET_LUA_STRING(f, 1);
	if(strlen(f.data) == 0)
	{
		ClearFilter(GetActFilter(gd));

		if(gd->selected_window == WINDOW_RIGHT)
		{
			if(gd->lstRight != NULL)
			{
				dlist_destroy(gd->lstRight);
				free(gd->lstRight);
			}
			gd->lstRight = ResetFilteredFileList(gd->lstFilterRight, gd->lstFullRight);
		}
		else
		{
			if(gd->lstLeft != NULL)
			{
				dlist_destroy(gd->lstLeft);
				free(gd->lstLeft);
			}
			gd->lstLeft = ResetFilteredFileList(gd->lstFilterLeft, gd->lstFullLeft);
		}
	}
	else
	{
		e = dlist_head(GetActFilter(gd));
		while(e != NULL)
		{
			dlist_remove(GetActFilter(gd), e, (void*)&x);
			free(x);
			e = dlist_head(GetActFilter(gd));
		}

		dlist_ins(GetActFilter(gd), strdup(f.data));
	}

	UpdateFilterList(gd, GetActFilter(gd), GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawFilter(gd);

	return 0;
}

/****f* LuaAPI/AddFilter
* FUNCTION
*	Applies another posix regex filter against the file list, without removing any from the stack.
* SYNOPSIS
AddFilter(filter)
* INPUTS
*	o filter (string) -- Posix regular expression
* RESULTS
*	o None
* SEE ALSO
*	SetFilter, AddGlob, SetGlob
* AUTHOR
*	Stu George
******
*/
int gme_AddFilter(lua_State *L)
{
	struct lstr f;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(f, 1);
	if(strlen(f.data) == 0)
	{
		ClearFilter(GetActFilter(gd));
		if(gd->selected_window == WINDOW_RIGHT)
		{
			if(gd->lstRight != NULL)
			{
				dlist_destroy(gd->lstRight);
				free(gd->lstRight);
			}
			gd->lstRight = ResetFilteredFileList(gd->lstFilterRight, gd->lstFullRight);
		}
		else
		{
			if(gd->lstLeft != NULL)
			{
				dlist_destroy(gd->lstLeft);
				free(gd->lstLeft);
			}
			gd->lstLeft = ResetFilteredFileList(gd->lstFilterLeft, gd->lstFullLeft);
		}
	}
	else
	{
		dlist_ins(GetActFilter(gd), strdup(f.data));
	}

	UpdateFilterList(gd, GetActFilter(gd), GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawFilter(gd);

	return 0;
}

/****f* LuaAPI/SetGlob
* FUNCTION
*	Removes all existing globs and applies a posix fnmatch glob against the file list.
* SYNOPSIS
SetGlob(glob)
* INPUTS
*	o glob (string) -- Posix glob
* RESULTS
*	o None
* SEE ALSO
*	AddGlob, SetFilter, AddFilter
* AUTHOR
*	Stu George
******
*/
int gme_SetGlob(lua_State *L)
{
	struct lstr f;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DLElement *e;
	char *x;

	GET_LUA_STRING(f, 1);

	if(strlen(f.data) == 0)
	{
		ClearGlob(GetActGlob(gd));
		if(gd->selected_window == WINDOW_RIGHT)
		{
			if(gd->lstRight != NULL)
			{
				dlist_destroy(gd->lstRight);
				free(gd->lstRight);
			}
			gd->lstRight = ResetFilteredFileList(gd->lstFilterRight, gd->lstFullRight);
		}
		else
		{
			if(gd->lstLeft != NULL)
			{
				dlist_destroy(gd->lstLeft);
				free(gd->lstLeft);
			}
			gd->lstLeft = ResetFilteredFileList(gd->lstFilterLeft, gd->lstFullLeft);
		}
	}
	else
	{
		e = dlist_head(GetActGlob(gd));
		while(e != NULL)
		{
			dlist_remove(GetActGlob(gd), e, (void*)&x);
			free(x);
			e = dlist_head(GetActGlob(gd));
		}

		dlist_ins(GetActGlob(gd), strdup(f.data));
	}

	//UpdateGlobList(gd, GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	UpdateFilterList(gd, GetActFilter(gd), GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawFilter(gd);

	return 0;
}

/****f* LuaAPI/AddGlob
* FUNCTION
*	Applies another posix glob against the file list, without removing any from the stack.
* SYNOPSIS
AddGlob(glob)
* INPUTS
*	o glob (string) -- Posix glob
* RESULTS
*	o None
* SEE ALSO
*	SetFilter, AddFilter, SetGlob
* AUTHOR
*	Stu George
******
*/
int gme_AddGlob(lua_State *L)
{
	struct lstr f;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(f, 1);

	if(strlen(f.data) == 0)
	{
		ClearGlob(GetActGlob(gd));
		if(gd->selected_window == WINDOW_RIGHT)
		{
			if(gd->lstRight != NULL)
			{
				dlist_destroy(gd->lstRight);
				free(gd->lstRight);
			}
			gd->lstRight = ResetFilteredFileList(gd->lstFilterRight, gd->lstFullRight);
		}
		else
		{
			if(gd->lstLeft != NULL)
			{
				dlist_destroy(gd->lstLeft);
				free(gd->lstLeft);
			}
			gd->lstLeft = ResetFilteredFileList(gd->lstFilterLeft, gd->lstFullLeft);
		}
	}
	else
	{
		dlist_ins(GetActGlob(gd), strdup(f.data));
	}

	//UpdateGlobList(gd, GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	UpdateFilterList(gd, GetActFilter(gd), GetActGlob(gd), GetActFullList(gd), GetActList(gd));
	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawFilter(gd);

	return 0;
}


/****f* LuaAPI/BindKey
* FUNCTION
*	Binds a key to a command string which is passed intnernally just as if you had typed
* 	it on the cli bar (eg: ":q")
* SYNOPSIS
error = BindKey(key, title, command)
* INPUTS
*	o key (constant) -- (need to insert key listing)
*	o title (string) -- what shows up on the command bar
*	o command (string) -- command string the key invokes
* RESULTS
*	error (integer):
*	o 0 -- OK
*	o -1 -- Key already bound
* SEE ALSO
*	SetFilter, AddFilter, SetGlob
* AUTHOR
*	Stu George
******
*/
int gme_BindKey(lua_State *L)
{
	struct lstr kstring;
	struct lstr ktitle;
	uint32_t key;
	uGlobalData *gd;
	uKeyBinding *kb;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	key = luaL_checknumber(L, 1);
	GET_LUA_STRING(ktitle, 2);
	GET_LUA_STRING(kstring, 3);

	kb = ScanKey(gd->lstHotKeys, key);
	if(kb != NULL)
	{
		lua_pushnumber(L, -1);
		return 1;
	}

	kb = malloc(sizeof(uKeyBinding));

	kb->key = key;
	kb->sCommand = strdup(kstring.data);
	kb->sTitle = strdup(ktitle.data);

	dlist_ins(gd->lstHotKeys, kb);

	lua_pushnumber(L, 0);
	return 1;
}


/****f* LuaAPI/ClearSortList
* FUNCTION
*	Clears the current 'sort' file list. If yo dont Add files to this you will have a blank window...
* SYNOPSIS
ClearSortList()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
* 	AddToSortList
* AUTHOR
*	Stu George
******
*/
int gme_ClearSortList(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DList *lstX;

	lstX = GetActList(gd);
	dlist_empty(lstX);

	return 0;
}

/****f* LuaAPI/AddToSortList
* FUNCTION
*	Adds a file to the sort list.
* SYNOPSIS
ClearSortList()
* INPUTS
*	o None
* RESULTS
*	o None
* SEE ALSO
* 	ClearSortList
* AUTHOR
*	Stu George
******
*/
int gme_AddToSortList(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	uDirEntry *de;
	struct lstr kname;

	GET_LUA_STRING(kname, 1);


	de = GetFileByName(GetActFullList(gd), kname.data);
	if(de != NULL)
	{
		dlist_ins(GetActList(gd), de);
	}

	return 0;
}


/****f* LuaAPI/RedrawWindow
* FUNCTION
*	Triggers a re-draw of the currently active window pane
* SYNOPSIS
RedrawWindow()
* INPUTS
*	o None
* RESULTS
*	o None
* AUTHOR
*	Stu George
******
*/
int gme_RedrawWindow(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));

	return 0;
}


int gme_ViewFile(lua_State *L)
{
	struct lstr name;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(name, 1);

	ViewFile(gd, name.data, NULL);
	DrawAll(gd);

	return 0;
}

int gme_QueueFileOp(lua_State *L)
{
	int op;
	struct lstr fname;
	uFileOperation *x;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	op = luaL_checknumber(L, 1);
	GET_LUA_STRING(fname, 2);

	if(op == eOp_Copy || op == eOp_Move || op == eOp_Delete)
	{
		x = calloc(1, sizeof(uFileOperation));

		x->type = op;
		switch(x->type)
		{
			case eOp_Delete:
				x->op.udtDelete.source_path = strdup(GetActDPath(gd));
				x->op.udtDelete.source_filename = strdup(fname.data);
				break;

			case eOp_Copy:
				x->op.udtCopy.source_path = strdup(GetActDPath(gd));
				x->op.udtCopy.source_filename = strdup(fname.data);
				x->op.udtCopy.dest_filename = strdup(fname.data);
				x->op.udtCopy.dest_path = strdup(GetInActDPath(gd));
				break;

			case eOp_Move:
				x->op.udtMove.source_path = strdup(GetActDPath(gd));
				x->op.udtMove.source_filename = strdup(fname.data);
				x->op.udtMove.dest_filename = strdup(fname.data);
				x->op.udtMove.dest_path = strdup(GetInActDPath(gd));
				break;

		}

		dlist_ins(gd->lstFileOps, x);
		lua_pushnumber(L, 0);
	}
	else
		lua_pushnumber(L, -1);

	return 1;
}


 int ViewOperationsLog(uGlobalData *gd, int intLine, uint8_t *buff, int len)
{
	DLElement *e;
	int j;
	uFileOperation *op;

	if(gd->lstFileOps == NULL)
		return -1;

	e = dlist_head(gd->lstFileOps);

	if(e == NULL)
		return -1;
	j = intLine;
	while( j > 0 && e != NULL)
	{
		e = dlist_next(e);
		j -= 1;
	}

	if(j > 0 || e == NULL)
		return -1;

	op = dlist_data(e);
	if(op == NULL || op->result_msg == NULL)
	{
		buff[0] = 0;
	}
	else if(op->result_code == 0)
	{
		switch(op->type)
		{
			case eOp_Delete:
				snprintf((char*)buff, len, "Delete OK on %s", op->op.udtDelete.source_filename);
				break;

			case eOp_Copy:
				snprintf((char*)buff, len, "Copy OK on %s to %s", op->op.udtCopy.source_filename, op->op.udtCopy.dest_path);
				break;

			case eOp_Move:
				snprintf((char*)buff, len, "Move OK on %s to %s", op->op.udtCopy.source_filename, op->op.udtCopy.dest_path);
				break;
		}
	}
	else
	{
		switch(op->type)
		{
			case eOp_Delete:
				snprintf((char*)buff, len, "Delete Failed on %s : %s", op->op.udtDelete.source_filename, op->result_msg);
				break;

			case eOp_Copy:
				snprintf((char*)buff, len, "Copy Failed on %s : %s", op->op.udtCopy.source_filename, op->result_msg);
				break;

			case eOp_Move:
				snprintf((char*)buff, len, "Move Failed on %s : %s", op->op.udtMove.source_filename, op->result_msg);
				break;
		}
	}

	return 0;
}



int gme_DoFileOps(lua_State *L)
{
	int err_count = 0;
	DLElement *e;
	uFileOperation *x;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	int i = 1;

	uDirEntry *de;
	char *high1, *high2;

	de = GetHighlightedFile(GetActList(gd), GetActWindow(gd)->highlight_line, GetActWindow(gd)->top_line);
	high1 = strdup(de->name);
	de = GetHighlightedFile(GetInActList(gd), GetInActWindow(gd)->highlight_line, GetInActWindow(gd)->top_line);
	high2 = strdup(de->name);


	e = dlist_head(gd->lstFileOps);

	lua_newtable(L);

	while(e != NULL)
	{
		x = dlist_data(e);

		x->result_code = 0;
		x->result_msg = NULL;

		switch(x->type)
		{
			case eOp_Delete:
				Ops_DeleteFile(gd, x);
				break;

			case eOp_Copy:
				Ops_CopyFile(gd, x);
				break;

			case eOp_Move:
				Ops_MoveFile(gd, x);
				break;
		}

		if(x->result_code != 0)
			err_count += 1;


		lua_pushnumber(L, i++);
		lua_newtable(L);

		lua_pushstring(L, "operation");
		lua_pushnumber(L, x->type);
		lua_settable(L, -3);

		lua_pushstring(L, "result_code");
		lua_pushnumber(L, x->result_code);
		lua_settable(L, -3);

		lua_pushstring(L, "result_msg");
		if(x->result_msg == NULL)
			lua_pushstring(L, "");
		else
			lua_pushstring(L, x->result_msg);
		lua_settable(L, -3);

		switch(x->type)
		{
			case eOp_Delete:
				lua_pushstring(L, "source_filename");
				lua_pushstring(L, x->op.udtDelete.source_filename);
				lua_settable(L, -3);

				lua_pushstring(L, "source_path");
				lua_pushstring(L, x->op.udtDelete.source_path);
				lua_settable(L, -3);
				break;

			case eOp_Copy:
				lua_pushstring(L, "source_filename");
				lua_pushstring(L, x->op.udtCopy.source_filename);
				lua_settable(L, -3);

				lua_pushstring(L, "source_path");
				lua_pushstring(L, x->op.udtCopy.source_path);
				lua_settable(L, -3);

				lua_pushstring(L, "dest_filename");
				lua_pushstring(L, x->op.udtCopy.dest_filename);
				lua_settable(L, -3);

				lua_pushstring(L, "dest_path");
				lua_pushstring(L, x->op.udtCopy.dest_path);
				lua_settable(L, -3);
				break;

			case eOp_Move:
				lua_pushstring(L, "source_filename");
				lua_pushstring(L, x->op.udtMove.source_filename);
				lua_settable(L, -3);

				lua_pushstring(L, "source_path");
				lua_pushstring(L, x->op.udtMove.source_path);
				lua_settable(L, -3);

				lua_pushstring(L, "dest_filename");
				lua_pushstring(L, x->op.udtMove.dest_filename);
				lua_settable(L, -3);

				lua_pushstring(L, "dest_path");
				lua_pushstring(L, x->op.udtMove.dest_path);
				lua_settable(L, -3);
				break;
		}

		lua_settable(L, -3);

		e = dlist_next(e);
	}

	if(err_count > 0)
	{
		ViewFile(gd, "OPERATIONS LOG", &ViewOperationsLog);
		DrawAll(gd);
	}

	dlist_empty(gd->lstFileOps);

	if(gd->selected_window == WINDOW_LEFT)
	{
		chdir(GetActDPath(gd));
		UpdateDir(gd, high1);
		gd->selected_window = WINDOW_RIGHT;
		chdir( GetActDPath(gd));
		UpdateDir(gd, high2);
		gd->selected_window = WINDOW_LEFT;
	}
	else
	{
		chdir(GetActDPath(gd));
		UpdateDir(gd, high1);
		gd->selected_window = WINDOW_LEFT;
		chdir(GetActDPath(gd));
		UpdateDir(gd, high2);
		gd->selected_window = WINDOW_RIGHT;
	}
	free(high1);
	free(high2);

	lua_pushnumber(L, err_count);
	return 2;
}

int gme_TagWithGlob(lua_State *L)
{
	struct lstr g;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(g, 1);

	lua_pushnumber(L, TagWithGlob(gd, g.data));

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

	return 1;
}

int gme_TagWithFilter(lua_State *L)
{
	struct lstr g;
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(g, 1);

	lua_pushnumber(L, TagWithFilter(gd, g.data));

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

	return 1;
}


int gme_ClearAllTags(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DLElement *e;
	uDirEntry *de;

	e = dlist_head(GetActList(gd));
	while(e != NULL)
	{
		de = dlist_data(e);
		e = dlist_next(e);

		de->tagged = 0;
	}

	GetActWindow(gd)->tagged_count = 0;

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

	return 0;
}


int gme_TagAll(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DLElement *e;
	uDirEntry *de;

	e = dlist_head(GetActList(gd));
	while(e != NULL)
	{
		de = dlist_data(e);
		e = dlist_next(e);

		de->tagged = 1;
	}

	GetActWindow(gd)->tagged_count = dlist_size(GetActList(gd));

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

	return 0;
}


int gme_TagFlip(lua_State *L)
{
	uGlobalData *gd;
	gd = GetGlobalData(L);
	assert(gd != NULL);
	DLElement *e;
	uDirEntry *de;

	int count = 0;

	e = dlist_head(GetActList(gd));
	while(e != NULL)
	{
		de = dlist_data(e);
		e = dlist_next(e);

		if(de->tagged == 0)
			count += 1;

		de->tagged ^= 1;
	}

	GetActWindow(gd)->tagged_count = count;

	DrawFileListWindow(GetActWindow(gd), GetActList(gd), GetActDPath(gd));
	DrawActive(gd);
	DrawStatusInfoLine(gd);

	return 0;
}
