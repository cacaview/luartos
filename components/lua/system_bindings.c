#include "system_bindings.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

// Include the new unified SD card driver header
#include "sdcard_driver.h"

static const char *TAG = "SYSTEM_BINDINGS";

// WiFi event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static bool s_wifi_initialized = false;
static bool s_wifi_connecting = false;

// Globals for async WiFi connection
static int s_wifi_connect_callback_ref = LUA_NOREF;
static lua_State* g_main_L = NULL;
static esp_timer_handle_t s_wifi_check_timer = NULL;

typedef struct {
    bool success;
    char msg[128];
} wifi_connect_result_t;

static volatile bool s_wifi_result_ready = false;
static wifi_connect_result_t s_wifi_result;

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (s_wifi_connecting) {
            esp_wifi_connect();
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_wifi_connecting && s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            s_wifi_connecting = false;
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_wifi_connecting = false;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// --- SD Card functions (Now wrappers for sdcard_driver) ---

int system_sd_init(lua_State* L) {
    // This function is now a placeholder. The actual init is in main.c
    // It now just checks the status.
    bool success = sdcard_is_mounted();
    if (success) {
        lua_pushboolean(L, true);
        lua_pushstring(L, "SD card already mounted");
        return 2;
    } else {
        lua_pushboolean(L, false);
        lua_pushstring(L, "SD card not mounted. Initialization should happen at startup.");
        return 2;
    }
}

int system_sd_is_mounted(lua_State* L) {
    lua_pushboolean(L, sdcard_is_mounted());
    return 1;
}

int system_sd_get_info(lua_State* L) {
    if (!sdcard_is_mounted()) {
        lua_pushnil(L);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    uint64_t total_bytes, used_bytes;
    esp_err_t err = sdcard_get_usage(&total_bytes, &used_bytes);

    if (err == ESP_OK) {
        lua_newtable(L);
        
        lua_pushstring(L, "total_bytes");
        lua_pushnumber(L, (double)total_bytes);
        lua_settable(L, -3);

        lua_pushstring(L, "used_bytes");
        lua_pushnumber(L, (double)used_bytes);
        lua_settable(L, -3);

        lua_pushstring(L, "free_bytes");
        lua_pushnumber(L, (double)(total_bytes - used_bytes));
        lua_settable(L, -3);
        
        lua_pushstring(L, "source");
        lua_pushstring(L, "sdcard_driver");
        lua_settable(L, -3);
        
        return 1;
    } else {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to get SD card usage: %s", esp_err_to_name(err));
        return 2;
    }
}

int system_sd_write_file(lua_State* L) {
    if (!sdcard_is_mounted()) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    const char* filename = luaL_checkstring(L, 1);
    const char* data = luaL_checkstring(L, 2);
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/sdcard/%s", filename);
    
    FILE* f = fopen(filepath, "w");
    if (f == NULL) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Failed to open file for writing");
        return 2;
    }
    
    fprintf(f, "%s", data);
    fclose(f);
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "File written successfully");
    return 2;
}

int system_sd_read_file(lua_State* L) {
    if (!sdcard_is_mounted()) {
        lua_pushnil(L);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    const char* filename = luaL_checkstring(L, 1);
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/sdcard/%s", filename);
    
    FILE* f = fopen(filepath, "r");
    if (f == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "Failed to open file for reading");
        return 2;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = malloc(fsize + 1);
    if (buffer == NULL) {
        fclose(f);
        lua_pushnil(L);
        lua_pushstring(L, "Failed to allocate memory");
        return 2;
    }
    
    fread(buffer, 1, fsize, f);
    buffer[fsize] = 0;
    fclose(f);
    
    lua_pushstring(L, buffer);
    free(buffer);
    return 1;
}

// --- WiFi functions (unchanged) ---
int system_wifi_init(lua_State* L) {
    if (s_wifi_initialized) {
        lua_pushboolean(L, true);
        lua_pushstring(L, "WiFi already initialized");
        return 2;
    }
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    s_wifi_initialized = true;
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "WiFi initialized successfully");
    return 2;
}

int system_wifi_scan(lua_State* L) {
    if (!s_wifi_initialized) {
        lua_pushnil(L);
        lua_pushstring(L, "WiFi not initialized");
        return 2;
    }

    esp_err_t ret = esp_wifi_scan_start(NULL, true);
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to start WiFi scan: %s", esp_err_to_name(ret));
        return 2;
    }

    vTaskDelay(pdMS_TO_TICKS(20));

    uint16_t ap_count = 0;
    ret = esp_wifi_scan_get_ap_num(&ap_count);
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to get AP number: %s", esp_err_to_name(ret));
        return 2;
    }

    if (ap_count == 0) {
        lua_newtable(L);
        return 1;
    }

    wifi_ap_record_t *ap_info = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_info == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to allocate memory for %d APs", ap_count);
        return 2;
    }

    ret = esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    if (ret != ESP_OK) {
        free(ap_info);
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to get AP records: %s", esp_err_to_name(ret));
        return 2;
    }

    lua_newtable(L);
    for (int i = 0; i < ap_count; i++) {
        lua_newtable(L);
        lua_pushstring(L, (char*)ap_info[i].ssid);
        lua_setfield(L, -2, "ssid");
        lua_pushinteger(L, ap_info[i].rssi);
        lua_setfield(L, -2, "rssi");
        lua_pushinteger(L, ap_info[i].authmode);
        lua_setfield(L, -2, "authmode");
        lua_rawseti(L, -2, i + 1);
    }

    free(ap_info);
    return 1;
}

static void wifi_connect_task(void* arg) {
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(30000));
    if (bits & WIFI_CONNECTED_BIT) {
        s_wifi_result.success = true;
        strncpy(s_wifi_result.msg, "Connected to WiFi", sizeof(s_wifi_result.msg) - 1);
    } else {
        s_wifi_result.success = false;
        strncpy(s_wifi_result.msg, "Failed to connect to WiFi", sizeof(s_wifi_result.msg) - 1);
        s_wifi_connecting = false;
    }
    s_wifi_result.msg[sizeof(s_wifi_result.msg) - 1] = '\0';
    s_wifi_result_ready = true;
    vTaskDelete(NULL);
}

static void check_wifi_result_callback(void* arg) {
    if (s_wifi_result_ready) {
        esp_timer_stop(s_wifi_check_timer);
        esp_timer_delete(s_wifi_check_timer);
        s_wifi_check_timer = NULL;
        if (g_main_L && s_wifi_connect_callback_ref != LUA_NOREF) {
            lua_State* co = lua_newthread(g_main_L);
            lua_rawgeti(g_main_L, LUA_REGISTRYINDEX, s_wifi_connect_callback_ref);
            lua_xmove(g_main_L, co, 1);
            lua_pushboolean(co, s_wifi_result.success);
            lua_pushstring(co, s_wifi_result.msg);
            int nres;
            int status = lua_resume(co, g_main_L, 2, &nres);
            if (status != LUA_OK && status != LUA_YIELD) {
                ESP_LOGE(TAG, "Lua WiFi callback error: %s", lua_tostring(co, -1) ? lua_tostring(co, -1) : "Unknown");
            }
            luaL_unref(g_main_L, LUA_REGISTRYINDEX, s_wifi_connect_callback_ref);
            s_wifi_connect_callback_ref = LUA_NOREF;
        }
        s_wifi_result_ready = false;
    }
}

int system_wifi_connect(lua_State* L) {
    if (!s_wifi_initialized) {
        luaL_error(L, "WiFi not initialized");
        return 0;
    }
    const char* ssid = luaL_checkstring(L, 1);
    const char* password = luaL_checkstring(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    if (s_wifi_check_timer) {
        esp_timer_stop(s_wifi_check_timer);
        esp_timer_delete(s_wifi_check_timer);
        s_wifi_check_timer = NULL;
    }
    if (s_wifi_connect_callback_ref != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, s_wifi_connect_callback_ref);
    }

    lua_pushvalue(L, 3);
    s_wifi_connect_callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    if (s_wifi_connecting) {
        esp_wifi_disconnect();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = strlen(password) == 0 ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    s_wifi_connecting = true;
    s_retry_num = 0;
    s_wifi_result_ready = false;
    
    xTaskCreate(wifi_connect_task, "wifi_connect_task", 4096, NULL, 5, NULL);

    esp_timer_create_args_t timer_args = {.callback = &check_wifi_result_callback, .name = "wifi_result_checker"};
    esp_timer_create(&timer_args, &s_wifi_check_timer);
    esp_timer_start_periodic(s_wifi_check_timer, 100 * 1000);

    esp_wifi_connect();
    
    return 0;
}

int system_wifi_disconnect(lua_State* L) {
    esp_wifi_disconnect();
    lua_pushboolean(L, true);
    return 1;
}

int system_wifi_is_connected(lua_State* L) {
    if (!s_wifi_initialized) {
        lua_pushboolean(L, false);
        return 1;
    }
    wifi_ap_record_t ap_info;
    lua_pushboolean(L, esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
    return 1;
}

int system_wifi_get_ip(lua_State* L) {
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_str[16];
        sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
        lua_pushstring(L, ip_str);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// --- System functions (unchanged) ---
int system_delay(lua_State* L) {
    vTaskDelay(pdMS_TO_TICKS(luaL_checkinteger(L, 1)));
    return 0;
}

int system_get_free_heap(lua_State* L) {
    lua_pushinteger(L, esp_get_free_heap_size());
    return 1;
}

int system_get_psram_size(lua_State* L) {
#ifdef CONFIG_SPIRAM
    lua_pushinteger(L, heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
#else
    lua_pushinteger(L, 0);
#endif
    return 1;
}

int system_restart(lua_State* L) {
    esp_restart();
    return 0;
}

// --- Timer functions (unchanged) ---
#define LUA_TIMER_METATABLE "lua_timer"

typedef struct {
    esp_timer_handle_t timer_handle;
    lua_State* g_L;
    int callback_ref;
    bool auto_reload;
    bool running;
} lua_timer_t;

static void timer_callback(void* arg) {
    lua_timer_t* timer = (lua_timer_t*)arg;
    if (!timer || !timer->g_L || timer->callback_ref == LUA_NOREF) return;
    lua_State* co = lua_newthread(timer->g_L);
    lua_rawgeti(timer->g_L, LUA_REGISTRYINDEX, timer->callback_ref);
    lua_xmove(timer->g_L, co, 1);
    int nres;
    int status = lua_resume(co, timer->g_L, 0, &nres);
    if (status != LUA_OK && status != LUA_YIELD) {
        ESP_LOGE(TAG, "Lua timer callback error: %s", lua_tostring(co, -1) ? lua_tostring(co, -1) : "Unknown");
    }
    if (!timer->auto_reload) timer->running = false;
}

static int timer_gc(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)luaL_checkudata(L, 1, LUA_TIMER_METATABLE);
    if (timer) {
        if (timer->running) esp_timer_stop(timer->timer_handle);
        esp_timer_delete(timer->timer_handle);
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        timer->callback_ref = LUA_NOREF;
        timer->running = false;
    }
    return 0;
}

int system_timer_create(lua_State* L) {
    uint32_t period_ms = luaL_checkinteger(L, 1);
    bool auto_reload = lua_toboolean(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    lua_timer_t* timer = (lua_timer_t*)lua_newuserdata(L, sizeof(lua_timer_t));
    luaL_getmetatable(L, LUA_TIMER_METATABLE);
    lua_setmetatable(L, -2);

    timer->g_L = L;
    timer->auto_reload = auto_reload;
    timer->running = false;
    lua_pushvalue(L, 3);
    timer->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    esp_timer_create_args_t timer_args = {.callback = &timer_callback, .arg = timer, .name = "lua_timer"};
    esp_err_t err = esp_timer_create(&timer_args, &timer->timer_handle);
    if (err != ESP_OK) {
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        return luaL_error(L, "Failed to create timer: %s", esp_err_to_name(err));
    }
    
    if (auto_reload) err = esp_timer_start_periodic(timer->timer_handle, (uint64_t)period_ms * 1000);
    else err = esp_timer_start_once(timer->timer_handle, (uint64_t)period_ms * 1000);
    
    if (err != ESP_OK) {
        esp_timer_delete(timer->timer_handle);
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        return luaL_error(L, "Failed to start timer: %s", esp_err_to_name(err));
    }
    timer->running = true;
    return 1;
}

int system_timer_stop(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)luaL_checkudata(L, 1, LUA_TIMER_METATABLE);
    if (timer && timer->running) {
        if (esp_timer_stop(timer->timer_handle) == ESP_OK) {
            timer->running = false;
            lua_pushboolean(L, true);
        } else {
            lua_pushboolean(L, false);
            return 1;
        }
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

// --- Function Registry ---
static const luaL_Reg system_functions[] = {
    // SD Card wrappers
    {"sd_init", system_sd_init},
    {"sd_is_mounted", system_sd_is_mounted},
    {"sd_get_info", system_sd_get_info},
    {"sd_write_file", system_sd_write_file},
    {"sd_read_file", system_sd_read_file},
    
    // WiFi functions
    {"wifi_init", system_wifi_init},
    {"wifi_scan", system_wifi_scan},
    {"wifi_connect", system_wifi_connect},
    {"wifi_disconnect", system_wifi_disconnect},
    {"wifi_is_connected", system_wifi_is_connected},
    {"wifi_get_ip", system_wifi_get_ip},
    
    // System functions
    {"delay", system_delay},
    {"get_free_heap", system_get_free_heap},
    {"get_psram_size", system_get_psram_size},
    {"restart", system_restart},
    
    // Timer functions
    {"timer_create", system_timer_create},
    {"timer_stop", system_timer_stop},
    
    {NULL, NULL}
};

int luaopen_system(lua_State* L) {
    ESP_LOGI(TAG, "Registering system bindings...");
    g_main_L = L;

    luaL_newmetatable(L, LUA_TIMER_METATABLE);
    lua_pushcfunction(L, timer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
    
    luaL_newlib(L, system_functions);
    
    ESP_LOGI(TAG, "System bindings registered successfully");
    return 1;
}
