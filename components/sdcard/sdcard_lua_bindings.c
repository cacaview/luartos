#include "sdcard_lua_bindings.h"
#include "sdcard_driver.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SDCARD_LUA";

// Lua callback structure for file listing
typedef struct {
    lua_State *L;
    int table_index;
    int count;
} lua_list_callback_data_t;

// Callback for file listing
static void lua_list_files_callback(const char* filename, size_t size, void* user_data)
{
    lua_list_callback_data_t *data = (lua_list_callback_data_t*)user_data;
    lua_State *L = data->L;
    
    // Create file info table
    lua_newtable(L);
    
    // Set filename
    lua_pushstring(L, "name");
    lua_pushstring(L, filename);
    lua_settable(L, -3);
    
    // Set size
    lua_pushstring(L, "size");
    lua_pushinteger(L, size);
    lua_settable(L, -3);
    
    // Add to main table
    lua_rawseti(L, data->table_index, ++data->count);
}

// sdcard.init()
static int lua_sdcard_init(lua_State *L)
{
    esp_err_t ret = sdcard_init();
    lua_pushboolean(L, ret == ESP_OK);
    if (ret != ESP_OK) {
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    return 1;
}

// sdcard.mount([mount_point], [max_files])
static int lua_sdcard_mount(lua_State *L)
{
    const char *mount_point = luaL_optstring(L, 1, NULL);
    int max_files = luaL_optinteger(L, 2, 5);
    
    esp_err_t ret = sdcard_mount(mount_point, max_files);
    lua_pushboolean(L, ret == ESP_OK);
    if (ret != ESP_OK) {
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    return 1;
}

// sdcard.unmount()
static int lua_sdcard_unmount(lua_State *L)
{
    esp_err_t ret = sdcard_unmount();
    lua_pushboolean(L, ret == ESP_OK);
    if (ret != ESP_OK) {
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    return 1;
}

// sdcard.is_mounted()
static int lua_sdcard_is_mounted(lua_State *L)
{
    lua_pushboolean(L, sdcard_is_mounted());
    return 1;
}

// sdcard.format()
static int lua_sdcard_format(lua_State *L)
{
    esp_err_t ret = sdcard_format();
    lua_pushboolean(L, ret == ESP_OK);
    if (ret != ESP_OK) {
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    return 1;
}

// sdcard.read_file(filename)
static int lua_sdcard_read_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    
    // Allocate buffer for file content
    char *buffer = malloc(8192);  // 8KB buffer
    if (!buffer) {
        lua_pushnil(L);
        lua_pushstring(L, "Out of memory");
        return 2;
    }
    
    int bytes_read = sdcard_read_file(filename, buffer, 8192);
    if (bytes_read < 0) {
        free(buffer);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to read file");
        return 2;
    }
    
    lua_pushlstring(L, buffer, bytes_read);
    free(buffer);
    return 1;
}

// sdcard.write_file(filename, data)
static int lua_sdcard_write_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    size_t data_len;
    const char *data = luaL_checklstring(L, 2, &data_len);
    
    int bytes_written = sdcard_write_file(filename, data, data_len);
    if (bytes_written < 0) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Failed to write file");
        return 2;
    }
    
    lua_pushboolean(L, true);
    lua_pushinteger(L, bytes_written);
    return 2;
}

// sdcard.delete_file(filename)
static int lua_sdcard_delete_file(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    
    esp_err_t ret = sdcard_delete_file(filename);
    lua_pushboolean(L, ret == ESP_OK);
    if (ret != ESP_OK) {
        lua_pushstring(L, "Failed to delete file");
        return 2;
    }
    return 1;
}

// sdcard.list_files([path])
static int lua_sdcard_list_files(lua_State *L)
{
    const char *path = luaL_optstring(L, 1, "/");
    
    // Create result table
    lua_newtable(L);
    int table_index = lua_gettop(L);
    
    lua_list_callback_data_t callback_data = {
        .L = L,
        .table_index = table_index,
        .count = 0
    };
    
    esp_err_t ret = sdcard_list_files(path, lua_list_files_callback, &callback_data);
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to list files");
        return 2;
    }
    
    return 1;
}

// sdcard.get_usage()
static int lua_sdcard_get_usage(lua_State *L)
{
    uint64_t total_bytes, used_bytes;
    esp_err_t ret = sdcard_get_usage(&total_bytes, &used_bytes);
    
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get disk usage");
        return 2;
    }
    
    // Create result table
    lua_newtable(L);
    
    lua_pushstring(L, "total");
    lua_pushinteger(L, total_bytes);
    lua_settable(L, -3);
    
    lua_pushstring(L, "used");
    lua_pushinteger(L, used_bytes);
    lua_settable(L, -3);
    
    lua_pushstring(L, "free");
    lua_pushinteger(L, total_bytes - used_bytes);
    lua_settable(L, -3);
    
    return 1;
}

// sdcard.get_info()
static int lua_sdcard_get_info(lua_State *L)
{
    sdmmc_card_t *card_info;
    esp_err_t ret = sdcard_get_info(&card_info);
    
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to get card info");
        return 2;
    }
    
    // Create result table
    lua_newtable(L);
    
    lua_pushstring(L, "name");
    lua_pushstring(L, card_info->cid.name);
    lua_settable(L, -3);
    
    lua_pushstring(L, "type");
    lua_pushstring(L, (card_info->ocr & (1 << 30)) ? "SDHC/SDXC" : "SDSC");  // Bit 30 indicates SDHC
    lua_settable(L, -3);
    
    lua_pushstring(L, "speed");
    lua_pushstring(L, (card_info->csd.tr_speed > 25000000) ? "high speed" : "default speed");
    lua_settable(L, -3);
    
    lua_pushstring(L, "capacity_mb");
    lua_pushinteger(L, ((uint64_t) card_info->csd.capacity) * card_info->csd.sector_size / (1024 * 1024));
    lua_settable(L, -3);
    
    lua_pushstring(L, "sector_size");
    lua_pushinteger(L, card_info->csd.sector_size);
    lua_settable(L, -3);
    
    return 1;
}

// SD card function table
static const luaL_Reg sdcard_funcs[] = {
    {"init", lua_sdcard_init},
    {"mount", lua_sdcard_mount},
    {"unmount", lua_sdcard_unmount},
    {"is_mounted", lua_sdcard_is_mounted},
    {"format", lua_sdcard_format},
    {"read_file", lua_sdcard_read_file},
    {"write_file", lua_sdcard_write_file},
    {"delete_file", lua_sdcard_delete_file},
    {"list_files", lua_sdcard_list_files},
    {"get_usage", lua_sdcard_get_usage},
    {"get_info", lua_sdcard_get_info},
    {NULL, NULL}
};

// Register SD card module
int luaopen_sdcard(lua_State *L)
{
    // Create sdcard table manually to avoid version issues
    lua_newtable(L);
    
    // Register functions
    const luaL_Reg *l = sdcard_funcs;
    for (; l->name != NULL; l++) {
        lua_pushcfunction(L, l->func);
        lua_setfield(L, -2, l->name);
    }
    
    // Add constants
    lua_pushstring(L, SDCARD_MOUNT_POINT);
    lua_setfield(L, -2, "MOUNT_POINT");
    
    return 1;
}
