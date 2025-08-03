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
#define LUA_PSRAM_MIN_SIZE 256      // Minimum size to allocate in PSRAM
#define LUA_LARGE_ALLOC_SIZE 1024   // Size considered "large"

void* lua_psram_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud; // Unused parameter
    
    // Handle free operation
    if (nsize == 0) {
        if (ptr != NULL) {
            // For free operation, we'll update statistics based on assumption
            // that larger allocations were likely in PSRAM
            if (osize >= LUA_PSRAM_MIN_SIZE && g_lua_memory_stats.psram_allocated >= osize) {
                g_lua_memory_stats.psram_allocated -= osize;
            } else if (g_lua_memory_stats.internal_allocated >= osize) {
                g_lua_memory_stats.internal_allocated -= osize;
            }
            
            if (g_lua_memory_stats.total_allocated >= osize) {
                g_lua_memory_stats.total_allocated -= osize;
            }
            
            g_lua_memory_stats.free_count++;
            
            free(ptr);
            
            ESP_LOGV(TAG, "Freed %zu bytes, total: %zu bytes", osize, g_lua_memory_stats.total_allocated);
        }
        return NULL;
    }
    
    void* new_ptr = NULL;
    
    // Handle new allocation or reallocation
    if (ptr == NULL) {
        // New allocation
        g_lua_memory_stats.alloc_count++;
        
#ifdef CONFIG_SPIRAM
        // Try PSRAM first for larger allocations
        if (nsize >= LUA_PSRAM_MIN_SIZE) {
            new_ptr = heap_caps_malloc(nsize, MALLOC_CAP_SPIRAM);
            if (new_ptr != NULL) {
                g_lua_memory_stats.psram_allocated += nsize;
                ESP_LOGV(TAG, "Allocated %zu bytes in PSRAM, PSRAM total: %zu bytes", 
                        nsize, g_lua_memory_stats.psram_allocated);
            }
        }
        
        // Fall back to internal RAM if PSRAM allocation failed or size is small
        if (new_ptr == NULL) {
#endif
            new_ptr = heap_caps_malloc(nsize, MALLOC_CAP_INTERNAL);
            if (new_ptr != NULL) {
                g_lua_memory_stats.internal_allocated += nsize;
                ESP_LOGV(TAG, "Allocated %zu bytes in internal RAM, internal total: %zu bytes", 
                        nsize, g_lua_memory_stats.internal_allocated);
            }
#ifdef CONFIG_SPIRAM
        }
#endif
        
        if (new_ptr != NULL) {
            g_lua_memory_stats.total_allocated += nsize;
            
            // Update peak usage
            if (g_lua_memory_stats.total_allocated > g_lua_memory_stats.peak_total) {
                g_lua_memory_stats.peak_total = g_lua_memory_stats.total_allocated;
            }
            if (g_lua_memory_stats.psram_allocated > g_lua_memory_stats.peak_psram) {
                g_lua_memory_stats.peak_psram = g_lua_memory_stats.psram_allocated;
            }
            if (g_lua_memory_stats.internal_allocated > g_lua_memory_stats.peak_internal) {
                g_lua_memory_stats.peak_internal = g_lua_memory_stats.internal_allocated;
            }
            
            if (nsize >= LUA_LARGE_ALLOC_SIZE) {
                ESP_LOGI(TAG, "Large allocation: %zu bytes, total Lua memory: %zu bytes", 
                        nsize, g_lua_memory_stats.total_allocated);
            }
        } else {
            ESP_LOGE(TAG, "Failed to allocate %zu bytes for Lua", nsize);
        }
        
    } else {
        // Reallocation
        bool old_is_psram = (osize >= LUA_PSRAM_MIN_SIZE);
        
        // Try to allocate new memory with same strategy as new allocation
#ifdef CONFIG_SPIRAM
        if (nsize >= LUA_PSRAM_MIN_SIZE) {
            new_ptr = heap_caps_malloc(nsize, MALLOC_CAP_SPIRAM);
        }
        if (new_ptr == NULL) {
#endif
            new_ptr = heap_caps_malloc(nsize, MALLOC_CAP_INTERNAL);
#ifdef CONFIG_SPIRAM
        }
#endif
        
        if (new_ptr != NULL) {
            // Copy old data
            size_t copy_size = (osize < nsize) ? osize : nsize;
            memcpy(new_ptr, ptr, copy_size);
            
            // Update statistics
            if (old_is_psram && g_lua_memory_stats.psram_allocated >= osize) {
                g_lua_memory_stats.psram_allocated -= osize;
            } else if (g_lua_memory_stats.internal_allocated >= osize) {
                g_lua_memory_stats.internal_allocated -= osize;
            }
            
            bool new_is_psram = (nsize >= LUA_PSRAM_MIN_SIZE);
#ifdef CONFIG_SPIRAM
            // Check if we actually got PSRAM
            if (new_is_psram) {
                // Try to determine if allocation succeeded in PSRAM
                size_t psram_free_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
                void* test_ptr = heap_caps_malloc(32, MALLOC_CAP_SPIRAM);
                size_t psram_free_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
                if (test_ptr) free(test_ptr);
                // If PSRAM is very low, assume we got internal RAM
                if (psram_free_before < 1024) {
                    new_is_psram = false;
                }
            }
#else
            new_is_psram = false;
#endif
            
            if (new_is_psram) {
                g_lua_memory_stats.psram_allocated += nsize;
            } else {
                g_lua_memory_stats.internal_allocated += nsize;
            }
            
            g_lua_memory_stats.total_allocated = g_lua_memory_stats.total_allocated - osize + nsize;
            
            // Free old memory
            free(ptr);
            
            ESP_LOGV(TAG, "Reallocated from %zu to %zu bytes, total: %zu bytes", 
                    osize, nsize, g_lua_memory_stats.total_allocated);
        } else {
            ESP_LOGE(TAG, "Failed to reallocate %zu bytes for Lua", nsize);
        }
    }
    
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
