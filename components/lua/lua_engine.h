#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "esp_log.h"
#include "lua_psram_alloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Lua engine with LVGL bindings
 * @return lua_State* The Lua state or NULL on error
 */
lua_State* lua_engine_init(void);

/**
 * @brief Deinitialize Lua engine
 * @param L Lua state to close
 */
void lua_engine_deinit(lua_State* L);

/**
 * @brief Execute a Lua script from string
 * @param L Lua state
 * @param script The Lua script string
 * @return int 0 on success, non-zero on error
 */
int lua_engine_exec_string(lua_State* L, const char* script);

/**
 * @brief Execute a Lua script from file
 * @param L Lua state  
 * @param filename Path to the Lua script file
 * @return int 0 on success, non-zero on error
 */
int lua_engine_exec_file(lua_State* L, const char* filename);

/**
 * @brief Call a Lua function by name
 * @param L Lua state
 * @param function_name Name of the function to call
 * @param nargs Number of arguments
 * @param nresults Number of results expected
 * @return int 0 on success, non-zero on error
 */
int lua_engine_call_function(lua_State* L, const char* function_name, int nargs, int nresults);

/**
 * @brief Get Lua memory usage statistics
 * @param L Lua state
 * @param total_alloc Total allocated memory
 * @param psram_alloc Memory allocated in PSRAM
 * @param internal_alloc Memory allocated in internal RAM
 */
void lua_engine_get_memory_stats(lua_State* L, size_t* total_alloc, size_t* psram_alloc, size_t* internal_alloc);

#ifdef __cplusplus
}
#endif

#endif // LUA_ENGINE_H
