#include "lua_psram_alloc.h"
#include <string.h>

static const char *TAG = "LUA_PSRAM_ALLOC";

// Memory usage tracking
typedef struct {
    size_t total_allocated;
    size_t psram_allocated;
    size_t internal_allocated;
    size_t peak_total;
    size_t peak_psram;
    size_t peak_internal;
    uint32_t alloc_count;
    uint32_t free_count;
} lua_memory_stats_t;

static lua_memory_stats_t g_lua_memory_stats = {0};

// Memory allocation thresholds
#define LUA_PSRAM_MIN_SIZE 16       // Lower threshold for PSRAM allocation
#define LUA_LARGE_ALLOC_SIZE 256    // Size considered "large"
#define LUA_FORCE_PSRAM_SIZE 16     // Force PSRAM for almost everything

// Simple, safe, and robust allocator using default heap capabilities.
void* lua_psram_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud; (void)osize; // Unused parameters

    if (nsize == 0) {
        heap_caps_free(ptr);
        return NULL;
    }

    // Use heap_caps_realloc_default which is smart about PSRAM.
    // It will try to allocate from PSRAM first if available and the size is appropriate.
    // This single call handles malloc (ptr=NULL), realloc, and free (nsize=0) correctly.
    void* new_ptr = heap_caps_realloc(ptr, nsize, MALLOC_CAP_DEFAULT);

    if (new_ptr == NULL && nsize > 0) {
        ESP_LOGE(TAG, "Failed to allocate or reallocate %zu bytes for Lua", nsize);
    }
    
    // Optional: Add simplified statistics tracking here if needed, but for now,
    // focus on stability. The complex tracking was a source of confusion.

    return new_ptr;
}

lua_State* lua_newstate_psram(void) {
    ESP_LOGI(TAG, "Creating Lua state with PSRAM allocator...");
    
    // Reset memory statistics
    memset(&g_lua_memory_stats, 0, sizeof(g_lua_memory_stats));
    
    // Create Lua state with custom allocator
    lua_State* L = lua_newstate(lua_psram_alloc, NULL);
    
    if (L != NULL) {
        ESP_LOGI(TAG, "Lua state created successfully with PSRAM allocator");
        
        // Log initial memory state
#ifdef CONFIG_SPIRAM
        ESP_LOGI(TAG, "Available PSRAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
        ESP_LOGI(TAG, "Available internal RAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    } else {
        ESP_LOGE(TAG, "Failed to create Lua state with PSRAM allocator");
    }
    
    return L;
}

void lua_get_memory_stats(lua_State* L, size_t* total_alloc, size_t* psram_alloc, size_t* internal_alloc) {
    (void)L; // Unused parameter
    
    if (total_alloc) *total_alloc = g_lua_memory_stats.total_allocated;
    if (psram_alloc) *psram_alloc = g_lua_memory_stats.psram_allocated;
    if (internal_alloc) *internal_alloc = g_lua_memory_stats.internal_allocated;
    
    ESP_LOGI(TAG, "Lua Memory Statistics:");
    ESP_LOGI(TAG, "  Total allocated: %zu bytes", g_lua_memory_stats.total_allocated);
    ESP_LOGI(TAG, "  PSRAM allocated: %zu bytes", g_lua_memory_stats.psram_allocated);
    ESP_LOGI(TAG, "  Internal allocated: %zu bytes", g_lua_memory_stats.internal_allocated);
    ESP_LOGI(TAG, "  Peak total: %zu bytes", g_lua_memory_stats.peak_total);
    ESP_LOGI(TAG, "  Peak PSRAM: %zu bytes", g_lua_memory_stats.peak_psram);
    ESP_LOGI(TAG, "  Peak internal: %zu bytes", g_lua_memory_stats.peak_internal);
    ESP_LOGI(TAG, "  Allocations: %u, Frees: %u", g_lua_memory_stats.alloc_count, g_lua_memory_stats.free_count);
}
