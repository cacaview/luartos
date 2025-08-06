#ifndef SYSTEM_BINDINGS_H
#define SYSTEM_BINDINGS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register system bindings in Lua state
 * @param L Lua state
 * @return int 0 on success, non-zero on error
 */
int luaopen_system(lua_State* L);

// SD Card functions
int system_sd_init(lua_State* L);
int system_sd_is_mounted(lua_State* L);
int system_sd_get_info(lua_State* L);
int system_sd_write_file(lua_State* L);
int system_sd_read_file(lua_State* L);

// WiFi functions
int system_wifi_init(lua_State* L);
int system_wifi_scan(lua_State* L);
int system_wifi_connect(lua_State* L);
int system_wifi_disconnect(lua_State* L);
int system_wifi_is_connected(lua_State* L);
int system_wifi_get_ip(lua_State* L);

// System functions
int system_delay(lua_State* L);
int system_get_free_heap(lua_State* L);
int system_get_psram_size(lua_State* L);
int system_restart(lua_State* L);

// Timer functions
int system_timer_create(lua_State* L);
int system_timer_start(lua_State* L);
int system_timer_stop(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_BINDINGS_H
