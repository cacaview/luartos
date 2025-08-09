#ifndef SYSTEM_BINDINGS_H
#define SYSTEM_BINDINGS_H

#include "lua.h"
#include <stdbool.h>

/**
 * @brief Registers the system library for the Lua state.
 *
 * @param L The Lua state.
 * @return int 0 on success.
 */
int luaopen_system(lua_State* L);

#endif // SYSTEM_BINDINGS_H
