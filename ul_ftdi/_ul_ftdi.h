#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#if defined(__linux__)
//	#include "lin/ftd2xx.h"
#elif defined(__WIN32__)
	#include "windows.h"
//	#include "win/ftd2xx.h"
#endif

LUALIB_API int luaopen__ul_ftdi(lua_State *L);

