#ifndef SDCARD_LUA_BINDINGS_H
#define SDCARD_LUA_BINDINGS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register SD card functions to Lua
 * @param L Lua state
 * @return int Number of values returned to Lua
 */
int luaopen_sdcard(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif // SDCARD_LUA_BINDINGS_H
