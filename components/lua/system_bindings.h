#ifndef SYSTEM_BINDINGS_H
#define SYSTEM_BINDINGS_H

#include "lua.h"
#include <stdbool.h>

/**
 * @brief Initializes the SD card from C code.
 * 
 * This function is safe to call multiple times. It ensures the underlying
 * hardware initialization is only performed once.
 * 
 * @return true if the SD card is mounted successfully, false otherwise.
 */
bool system_bindings_init_sdcard(void);

/**
 * @brief Registers the system library for the Lua state.
 *
 * @param L The Lua state.
 * @return int 0 on success.
 */
int luaopen_system(lua_State* L);

#endif // SYSTEM_BINDINGS_H