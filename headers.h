// triggers mingw include

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <regex.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __WIN32__
#include <windows.h>
#endif


// Include path found by rantfile.. set with -I
#include <lauxlib.h>
#include <lualib.h>

#include "dlist.h"
#include "ini.h"
#include "logwrite.h"

#include "main.h"
#include "portability.h"

#include "version.h"
#include "lua_helper.h"
#include "lua_api.h"
#include "options.h"

#include "viewer.h"
#include "lua_helper_viewer.h"

#include "vblist.h"
#include "lua_helper_vblist.h"

#include "lua_common.h"
#include "lua_common_api.h"
#include "lua_hash.h"
#include "operations.h"
#include "help.h"
#include "menu.h"
#include "tree.h"

// screen config
#ifdef DRV_NCURSES
#include "ncurses_interface.h"
#endif
#ifdef DRV_GUI
#include "gui_interface.h"
#endif



#ifdef __MINGW_H
#ifndef __WORDSIZE
#define __WORDSIZE 32
#endif
#endif

// maps for linux usage
#define stricmp strcasecmp

