#ifndef LUA_PSRAM_ALLOC_H
#define LUA_PSRAM_ALLOC_H

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Custom memory allocator for Lua that uses PSRAM when available
 * @param ud User data (not used)
 * @param ptr Pointer to memory block to reallocate (NULL for new allocation)
 * @param osize Old size of memory block
 * @param nsize New size of memory block (0 to free)
 * @return void* Pointer to allocated memory or NULL
 */
void* lua_psram_alloc(void *ud, void *ptr, size_t osize, size_t nsize);

/**
 * @brief Initialize Lua state with PSRAM-aware allocator
 * @return lua_State* New Lua state or NULL on error
 */
lua_State* lua_newstate_psram(void);

/**
 * @brief Get memory usage statistics for Lua
 * @param L Lua state
 * @param total_alloc Total allocated memory
 * @param psram_alloc Memory allocated in PSRAM
 * @param internal_alloc Memory allocated in internal RAM
 */
void lua_get_memory_stats(lua_State* L, size_t* total_alloc, size_t* psram_alloc, size_t* internal_alloc);

#ifdef __cplusplus
}
#endif

#endif // LUA_PSRAM_ALLOC_H
