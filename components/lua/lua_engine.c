#include "lua_engine.h"
#include "lvgl_bindings.h"
#include "system_bindings.h"
#include <string.h>

static const char *TAG = "LUA_ENGINE";

lua_State* lua_engine_init(void) {
    ESP_LOGI(TAG, "Initializing Lua engine with PSRAM support...");
    
    // Log memory before Lua initialization
    ESP_LOGI(TAG, "Memory before Lua init:");
    ESP_LOGI(TAG, "  Free internal RAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "  Free PSRAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "  Total PSRAM: %zu bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
#endif
    
    // Create new Lua state with PSRAM-aware allocator
    lua_State* L = lua_newstate_psram();
    if (L == NULL) {
        ESP_LOGE(TAG, "Failed to create Lua state with PSRAM allocator");
        return NULL;
    }
    
    // Open standard Lua libraries
    ESP_LOGI(TAG, "Opening standard Lua libraries...");
    luaL_openlibs(L);
    
    // Log memory after opening libraries
    size_t total_alloc, psram_alloc, internal_alloc;
    lua_get_memory_stats(L, &total_alloc, &psram_alloc, &internal_alloc);
    
    // Register LVGL bindings
    ESP_LOGI(TAG, "Registering LVGL bindings...");
    if (luaopen_lvgl(L) != 0) {
        ESP_LOGE(TAG, "Failed to register LVGL bindings");
        lua_close(L);
        return NULL;
    }
    
    // Register system bindings
    ESP_LOGI(TAG, "Registering system bindings...");
    if (luaopen_system(L) != 0) {
        ESP_LOGE(TAG, "Failed to register system bindings");
        lua_close(L);
        return NULL;
    }
    
    // Log final memory usage
    ESP_LOGI(TAG, "Lua engine initialized successfully");
    lua_get_memory_stats(L, &total_alloc, &psram_alloc, &internal_alloc);
    
    ESP_LOGI(TAG, "Memory after Lua init:");
    ESP_LOGI(TAG, "  Free internal RAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "  Free PSRAM: %zu bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
    
    return L;
}

void lua_engine_deinit(lua_State* L) {
    if (L != NULL) {
        ESP_LOGI(TAG, "Closing Lua engine...");
        lua_close(L);
        ESP_LOGI(TAG, "Lua engine closed");
    }
}

int lua_engine_exec_string(lua_State* L, const char* script) {
    if (L == NULL || script == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for exec_string");
        return -1;
    }
    
    ESP_LOGI(TAG, "Executing Lua script string...");
    
    // Load and compile the script
    int load_result = luaL_loadstring(L, script);
    if (load_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ESP_LOGE(TAG, "Failed to load script: %s", error);
        lua_pop(L, 1); // Remove error message
        return load_result;
    }
    
    // Execute the script
    int exec_result = lua_pcall(L, 0, 0, 0);
    if (exec_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ESP_LOGE(TAG, "Failed to execute script: %s", error);
        lua_pop(L, 1); // Remove error message
        return exec_result;
    }
    
    ESP_LOGI(TAG, "Script executed successfully");
    return 0;
}

int lua_engine_exec_file(lua_State* L, const char* filename) {
    if (L == NULL || filename == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for exec_file");
        return -1;
    }
    
    ESP_LOGI(TAG, "Executing Lua script file: %s", filename);
    
    // Load and compile the file
    int load_result = luaL_loadfile(L, filename);
    if (load_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ESP_LOGE(TAG, "Failed to load file %s: %s", filename, error);
        lua_pop(L, 1); // Remove error message
        return load_result;
    }
    
    // Execute the script
    int exec_result = lua_pcall(L, 0, 0, 0);
    if (exec_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ESP_LOGE(TAG, "Failed to execute file %s: %s", filename, error);
        lua_pop(L, 1); // Remove error message
        return exec_result;
    }
    
    ESP_LOGI(TAG, "File executed successfully");
    return 0;
}

int lua_engine_call_function(lua_State* L, const char* function_name, int nargs, int nresults) {
    if (L == NULL || function_name == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for call_function");
        return -1;
    }
    
    ESP_LOGI(TAG, "Calling Lua function: %s", function_name);
    
    // Get the function from global scope
    lua_getglobal(L, function_name);
    if (!lua_isfunction(L, -1)) {
        ESP_LOGE(TAG, "Function %s not found or not a function", function_name);
        lua_pop(L, 1);
        return -1;
    }
    
    // Arguments should already be on the stack
    
    // Call the function
    int call_result = lua_pcall(L, nargs, nresults, 0);
    if (call_result != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        ESP_LOGE(TAG, "Failed to call function %s: %s", function_name, error);
        lua_pop(L, 1); // Remove error message
        return call_result;
    }
    
    ESP_LOGI(TAG, "Function called successfully");
    return 0;
}

void lua_engine_get_memory_stats(lua_State* L, size_t* total_alloc, size_t* psram_alloc, size_t* internal_alloc) {
    if (L != NULL) {
        lua_get_memory_stats(L, total_alloc, psram_alloc, internal_alloc);
    } else {
        if (total_alloc) *total_alloc = 0;
        if (psram_alloc) *psram_alloc = 0;
        if (internal_alloc) *internal_alloc = 0;
    }
}
