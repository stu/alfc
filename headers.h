#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <regex.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "memwatch.h"

// default for mingw..
#ifdef __MINGW_H
#include <lua/lauxlib.h>
#include <lua/lualib.h>
#else
// default for ubuntu..
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#endif

#include "stucore/dlist.h"
#include "stucore/ini.h"
#include "stucore/logwrite.h"

#include "main.h"
#include "version.h"
#include "lua_helper.h"
#include "lua_api.h"
#include "options.h"

// screen config
#include "ncurses_interface.h"
