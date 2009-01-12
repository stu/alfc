#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "memwatch.h"

// default for ubuntu..
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>

#include "stucore/dlist.h"
#include "stucore/ini.h"
#include "stucore/logwrite.h"

#include "main.h"
#include "version.h"
#include "lua_helper.h"
#include "lua_api.h"
#include "options.h"

