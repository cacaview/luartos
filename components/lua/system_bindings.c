#include "system_bindings.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#ifdef CONFIG_SPIRAM
// In newer ESP-IDF versions, PSRAM functions might be in different headers
// #include "esp_psram.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include <string.h>

static const char *TAG = "SYSTEM_BINDINGS";

// WiFi event bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static bool s_wifi_initialized = false;

// SD Card mount point
#define MOUNT_POINT "/sdcard"
static bool s_sd_mounted = false;
static bool s_wifi_connecting = false;

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Only connect if we're in connecting mode
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

// SD Card functions
int system_sd_init(lua_State* L) {
    esp_err_t ret;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    
    ESP_LOGI(TAG, "Initializing SD card");
    
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;
    
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s)", esp_err_to_name(ret));
        }
        lua_pushboolean(L, false);
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    
    ESP_LOGI(TAG, "SD card initialized successfully");
    s_sd_mounted = true;
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "SD card mounted successfully");
    return 2;
}

int system_sd_is_mounted(lua_State* L) {
    lua_pushboolean(L, s_sd_mounted);
    return 1;
}

int system_sd_get_info(lua_State* L) {
    if (!s_sd_mounted) {
        lua_pushnil(L);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    
    if (f_getfree("0:", &fre_clust, &fs) == FR_OK) {
        tot_sect = (fs->n_fatent - 2) * fs->csize;
        fre_sect = fre_clust * fs->csize;
        
        lua_newtable(L);
        lua_pushinteger(L, tot_sect * 512);
        lua_setfield(L, -2, "total_bytes");
        lua_pushinteger(L, fre_sect * 512);
        lua_setfield(L, -2, "free_bytes");
        lua_pushinteger(L, (tot_sect - fre_sect) * 512);
        lua_setfield(L, -2, "used_bytes");
        
        return 1;
    }
    
    lua_pushnil(L);
    lua_pushstring(L, "Failed to get SD card info");
    return 2;
}

int system_sd_write_file(lua_State* L) {
    if (!s_sd_mounted) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    const char* filename = luaL_checkstring(L, 1);
    const char* data = luaL_checkstring(L, 2);
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", MOUNT_POINT, filename);
    
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
    if (!s_sd_mounted) {
        lua_pushnil(L);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    const char* filename = luaL_checkstring(L, 1);
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", MOUNT_POINT, filename);
    
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

// WiFi functions
int system_wifi_init(lua_State* L) {
    if (s_wifi_initialized) {
        lua_pushboolean(L, true);
        lua_pushstring(L, "WiFi already initialized");
        return 2;
    }
    
    // Initialize NVS first (required for WiFi)
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
    
    // Don't auto-connect, wait for explicit connection request
    
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
    
    esp_wifi_scan_start(NULL, true);
    
    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    
    if (ap_count == 0) {
        lua_newtable(L);
        return 1;
    }
    
    wifi_ap_record_t *ap_info = malloc(sizeof(wifi_ap_record_t) * ap_count);
    esp_wifi_scan_get_ap_records(&ap_count, ap_info);
    
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

int system_wifi_connect(lua_State* L) {
    if (!s_wifi_initialized) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "WiFi not initialized");
        return 2;
    }
    
    const char* ssid = luaL_checkstring(L, 1);
    const char* password = luaL_checkstring(L, 2);
    
    // Disconnect first if already connecting/connected
    if (s_wifi_connecting) {
        esp_wifi_disconnect();
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait a bit
        s_wifi_connecting = false;
    }
    
    wifi_config_t wifi_config = {0};
    
    // Set authentication mode based on password
    if (strlen(password) == 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    } else {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    
    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi config: %s", esp_err_to_name(ret));
        lua_pushboolean(L, false);
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    
    // Set connecting flag and reset retry counter
    s_wifi_connecting = true;
    s_retry_num = 0;
    
    esp_wifi_connect();
    
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        lua_pushboolean(L, true);
        lua_pushstring(L, "Connected to WiFi");
    } else if (bits & WIFI_FAIL_BIT) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Failed to connect to WiFi");
    } else {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Unexpected event");
    }
    
    return 2;
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
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    lua_pushboolean(L, ret == ESP_OK);
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

// System functions
int system_delay(lua_State* L) {
    uint32_t ms = luaL_checkinteger(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}

int system_get_free_heap(lua_State* L) {
    lua_pushinteger(L, esp_get_free_heap_size());
    return 1;
}

int system_get_psram_size(lua_State* L) {
#ifdef CONFIG_SPIRAM
    // Use heap_caps to get total SPIRAM size (free + used)
    size_t total_spiram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    lua_pushinteger(L, total_spiram);
#else
    lua_pushinteger(L, 0);
#endif
    return 1;
}

int system_restart(lua_State* L) {
    esp_restart();
    return 0;
}

// Timer functions
typedef struct {
    esp_timer_handle_t timer;
    lua_State* L;
    int callback_ref;
    uint32_t period_ms;
    bool auto_reload;
} lua_timer_t;

static void timer_callback(void* arg) {
    lua_timer_t* timer = (lua_timer_t*)arg;
    if (timer->L && timer->callback_ref != LUA_NOREF) {
        lua_rawgeti(timer->L, LUA_REGISTRYINDEX, timer->callback_ref);
        if (lua_isfunction(timer->L, -1)) {
            lua_call(timer->L, 0, 0);
        } else {
            lua_pop(timer->L, 1);
        }
    }
}

int system_timer_create(lua_State* L) {
    uint32_t period_ms = luaL_checkinteger(L, 1);
    bool auto_reload = lua_toboolean(L, 2);
    
    if (!lua_isfunction(L, 3)) {
        luaL_error(L, "Third argument must be a function");
        return 0;
    }
    
    lua_timer_t* timer = (lua_timer_t*)lua_newuserdata(L, sizeof(lua_timer_t));
    timer->L = L;
    
    // Store callback function in registry
    lua_pushvalue(L, 3); // Copy function to top of stack
    timer->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    // Create ESP timer
    esp_timer_create_args_t timer_args = {
        .callback = timer_callback,
        .arg = timer,
        .name = "lua_timer"
    };
    
    esp_err_t err = esp_timer_create(&timer_args, &timer->timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer: %s", esp_err_to_name(err));
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        lua_pushnil(L);
        return 1;
    }
    
    // Store timer configuration for later use
    timer->period_ms = period_ms;
    timer->auto_reload = auto_reload;
    
    lua_pushlightuserdata(L, timer);
    return 1;
}

int system_timer_start(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)lua_touserdata(L, 1);
    if (!timer) {
        luaL_error(L, "Invalid timer");
        return 0;
    }
    
    uint64_t period_us = timer->period_ms * 1000; // Convert ms to us
    
    esp_err_t err;
    if (timer->auto_reload) {
        err = esp_timer_start_periodic(timer->timer, period_us);
    } else {
        err = esp_timer_start_once(timer->timer, period_us);
    }
    
    lua_pushboolean(L, err == ESP_OK);
    return 1;
}

int system_timer_stop(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)lua_touserdata(L, 1);
    if (!timer) {
        luaL_error(L, "Invalid timer");
        return 0;
    }
    
    esp_timer_stop(timer->timer);
    lua_pushboolean(L, true);
    return 1;
}

// Function registry
static const luaL_Reg system_functions[] = {
    // SD Card functions
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
    {"timer_start", system_timer_start},
    {"timer_stop", system_timer_stop},
    
    {NULL, NULL}
};

int luaopen_system(lua_State* L) {
    ESP_LOGI(TAG, "Registering system bindings...");
    
    // Create system table
    luaL_newlib(L, system_functions);
    
    // Set global system table
    lua_setglobal(L, "system");
    
    ESP_LOGI(TAG, "System bindings registered successfully");
    return 0;
}
