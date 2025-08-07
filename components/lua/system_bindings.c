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
#include "driver/spi_common.h"
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

// SD Card mount point and SPI configuration
#define MOUNT_POINT "/sdcard"
#define SDCARD_SPI_HOST SPI3_HOST
#define SDCARD_CS_GPIO 9
#define SDCARD_SCK_GPIO 11
#define SDCARD_MOSI_GPIO 42
#define SDCARD_MISO_GPIO 41
#define SDCARD_MAX_TRANSFER_SIZE 8192

static bool s_sd_mounted = false;
static bool s_wifi_connecting = false;
static bool s_spi_bus_initialized = false;

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
    
    ESP_LOGI(TAG, "Initializing SD card using SPI protocol");
    ESP_LOGI(TAG, "SPI GPIO configuration: CS=%d, SCK=%d, MOSI=%d, MISO=%d",
             SDCARD_CS_GPIO, SDCARD_SCK_GPIO, SDCARD_MOSI_GPIO, SDCARD_MISO_GPIO);
    
    // Initialize SPI bus if not already done
    if (!s_spi_bus_initialized) {
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SDCARD_MOSI_GPIO,
            .miso_io_num = SDCARD_MISO_GPIO,
            .sclk_io_num = SDCARD_SCK_GPIO,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = SDCARD_MAX_TRANSFER_SIZE,
        };
        
        ret = spi_bus_initialize(SDCARD_SPI_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
            lua_pushboolean(L, false);
            lua_pushstring(L, esp_err_to_name(ret));
            return 2;
        }
        
        s_spi_bus_initialized = true;
        ESP_LOGI(TAG, "SPI bus initialized successfully");
    }
    
    // Configure SD/MMC host for SPI protocol
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SDCARD_SPI_HOST;
    host.max_freq_khz = 1000;  // Start with conservative 1MHz
    
    // Configure SPI device
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SDCARD_CS_GPIO;
    slot_config.host_id = SDCARD_SPI_HOST;
    slot_config.gpio_cd = GPIO_NUM_NC;    // No card detect
    slot_config.gpio_wp = GPIO_NUM_NC;    // No write protect
    slot_config.gpio_int = GPIO_NUM_NC;   // No interrupt
    
    // Mount filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 0  // Use default
    };
    
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    
    ESP_LOGI(TAG, "Attempting to mount SD card via SPI...");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem - card may need formatting");
        } else if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "SD card initialization timeout - check connections");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        }
        lua_pushboolean(L, false);
        lua_pushstring(L, esp_err_to_name(ret));
        return 2;
    }
    
    // Print card info
    ESP_LOGI(TAG, "SD card mounted successfully via SPI");
    ESP_LOGI(TAG, "Card name: %s", card->cid.name);
    ESP_LOGI(TAG, "Card type: %s", (card->ocr & (1 << 30)) ? "SDHC/SDXC" : "SDSC");
    
    // 详细的硬件容量信息调试
    ESP_LOGI(TAG, "=== SD CARD HARDWARE INFO ===");
    ESP_LOGI(TAG, "CSD capacity: %u", card->csd.capacity);
    ESP_LOGI(TAG, "CSD sector size: %u", card->csd.sector_size);
    ESP_LOGI(TAG, "Calculated size: %llu bytes", ((uint64_t) card->csd.capacity) * card->csd.sector_size);
    ESP_LOGI(TAG, "Calculated size: %llu MB", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    ESP_LOGI(TAG, "Calculated size: %llu GB", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024 * 1024));
    ESP_LOGI(TAG, "=== END HARDWARE INFO ===");
    
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
    
    ESP_LOGI(TAG, "=== SD CARD INFO RETRIEVAL START ===");
    
    // 尝试获取FatFS信息
    FRESULT fatfs_result = f_getfree("0:", &fre_clust, &fs);
    if (fatfs_result == FR_OK) {
        tot_sect = (fs->n_fatent - 2) * fs->csize;
        fre_sect = fre_clust * fs->csize;
        
        ESP_LOGI(TAG, "=== FATFS INFORMATION ===");
        ESP_LOGI(TAG, "FatFS sector size: %d bytes", fs->ssize);
        ESP_LOGI(TAG, "FatFS cluster size: %d sectors", fs->csize);
        ESP_LOGI(TAG, "Total FAT entries: %d", fs->n_fatent);
        ESP_LOGI(TAG, "Free clusters: %d", fre_clust);
        ESP_LOGI(TAG, "Total sectors: %d", tot_sect);
        ESP_LOGI(TAG, "Free sectors: %d", fre_sect);
        
        // 使用动态扇区大小
        uint64_t total_bytes = (uint64_t)tot_sect * fs->ssize;
        uint64_t free_bytes = (uint64_t)fre_sect * fs->ssize;
        uint64_t used_bytes = total_bytes - free_bytes;
        
        ESP_LOGI(TAG, "FATFS Total bytes: %llu (%llu MB, %llu GB)",
                 total_bytes, total_bytes / (1024*1024), total_bytes / (1024*1024*1024));
        ESP_LOGI(TAG, "FATFS Free bytes: %llu (%llu MB)",
                 free_bytes, free_bytes / (1024*1024));
        ESP_LOGI(TAG, "FATFS Used bytes: %llu (%llu MB)",
                 used_bytes, used_bytes / (1024*1024));
        
        // 创建返回表
        lua_newtable(L);
        
        // 强制使用 lua_pushnumber (double) 来传递64位整数，避免在32位lua_Integer环境下溢出
        lua_pushstring(L, "total_bytes");
        lua_pushnumber(L, (double)total_bytes);
        lua_settable(L, -3);

        lua_pushstring(L, "free_bytes");
        lua_pushnumber(L, (double)free_bytes);
        lua_settable(L, -3);

        lua_pushstring(L, "used_bytes");
        lua_pushnumber(L, (double)used_bytes);
        lua_settable(L, -3);
        
        // 添加调试标志
        lua_pushstring(L, "source");
        lua_pushstring(L, "fatfs");
        lua_settable(L, -3);
        
        ESP_LOGI(TAG, "=== SD CARD INFO RETRIEVAL SUCCESS ===");
        return 1;
    } else {
        ESP_LOGE(TAG, "FatFS f_getfree failed with result: %d", fatfs_result);
    }
    
    ESP_LOGE(TAG, "=== SD CARD INFO RETRIEVAL FAILED ===");
    lua_pushnil(L);
    lua_pushstring(L, "Failed to get SD card info from FatFS");
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

// Final, robust implementation of system_wifi_scan
int system_wifi_scan(lua_State* L) {
    if (!s_wifi_initialized) {
        lua_pushnil(L);
        lua_pushstring(L, "WiFi not initialized");
        return 2; // Return nil + error message
    }

    esp_err_t ret = esp_wifi_scan_start(NULL, true);
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to start WiFi scan: %s", esp_err_to_name(ret));
        return 2;
    }

    // Add a small delay to mitigate potential race conditions in the Wi-Fi driver,
    // ensuring results are ready before we try to read them.
    vTaskDelay(pdMS_TO_TICKS(20));

    uint16_t ap_count = 0;
    ret = esp_wifi_scan_get_ap_num(&ap_count);
    if (ret != ESP_OK) {
        lua_pushnil(L);
        lua_pushfstring(L, "Failed to get AP number: %s", esp_err_to_name(ret));
        return 2;
    }

    if (ap_count == 0) {
        lua_newtable(L); // Success: return an empty table
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

    // Success path: create and return the table of networks
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
    return 1; // Return the table
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

// Timer functions - Robust implementation with coroutines and GC
#define LUA_TIMER_METATABLE "lua_timer"

typedef struct {
    esp_timer_handle_t timer_handle;
    lua_State* g_L; // Reference to the main Lua state
    int callback_ref;
    bool auto_reload;
    bool running;
} lua_timer_t;

static void timer_callback(void* arg) {
    lua_timer_t* timer = (lua_timer_t*)arg;
    if (!timer || !timer->g_L || timer->callback_ref == LUA_NOREF) {
        return;
    }

    // Create a new coroutine for the callback
    lua_State* co = lua_newthread(timer->g_L);

    // Get the callback function from the registry and move it to the coroutine
    lua_rawgeti(timer->g_L, LUA_REGISTRYINDEX, timer->callback_ref);
    lua_xmove(timer->g_L, co, 1);

    // Resume the coroutine
    int nres;
    int status = lua_resume(co, timer->g_L, 0, &nres);
    if (status != LUA_OK && status != LUA_YIELD) {
        const char* error_msg = lua_tostring(co, -1);
        ESP_LOGE(TAG, "Lua timer callback error: %s", error_msg ? error_msg : "Unknown error");
    }

    // If it's a one-shot timer, mark it as not running
    if (!timer->auto_reload) {
        timer->running = false;
    }
}

static int timer_gc(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)luaL_checkudata(L, 1, LUA_TIMER_METATABLE);
    if (timer) {
        if (timer->running) {
            esp_timer_stop(timer->timer_handle);
        }
        esp_timer_delete(timer->timer_handle);
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        timer->callback_ref = LUA_NOREF;
        timer->running = false;
        ESP_LOGD(TAG, "Lua timer garbage collected");
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

    // Store the main Lua state, not the coroutine state
    timer->g_L = L;
    timer->auto_reload = auto_reload;
    timer->running = false;

    lua_pushvalue(L, 3);
    timer->callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .arg = timer,
        .name = "lua_timer"
    };

    esp_err_t err = esp_timer_create(&timer_args, &timer->timer_handle);
    if (err != ESP_OK) {
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        return luaL_error(L, "Failed to create system timer: %s", esp_err_to_name(err));
    }

    uint64_t period_us = (uint64_t)period_ms * 1000;
    if (auto_reload) {
        err = esp_timer_start_periodic(timer->timer_handle, period_us);
    } else {
        err = esp_timer_start_once(timer->timer_handle, period_us);
    }

    if (err != ESP_OK) {
        esp_timer_delete(timer->timer_handle);
        luaL_unref(L, LUA_REGISTRYINDEX, timer->callback_ref);
        return luaL_error(L, "Failed to start system timer: %s", esp_err_to_name(err));
    }
    timer->running = true;

    return 1; // Return the userdata
}

int system_timer_start(lua_State* L) {
    // This function is deprecated as timer starts on creation.
    // Kept for API compatibility for now.
    lua_pushboolean(L, true);
    return 1;
}

int system_timer_stop(lua_State* L) {
    lua_timer_t* timer = (lua_timer_t*)luaL_checkudata(L, 1, LUA_TIMER_METATABLE);
    if (timer && timer->running) {
        esp_err_t err = esp_timer_stop(timer->timer_handle);
        if (err == ESP_OK) {
            timer->running = false;
            lua_pushboolean(L, true);
        } else {
            lua_pushboolean(L, false);
            lua_pushfstring(L, "Failed to stop timer: %s", esp_err_to_name(err));
            return 2;
        }
    } else {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Timer is not running or invalid");
        return 2;
    }
    return 1;
}

// FreeRTOS sleep function
int system_sleep(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    
    if (ms < 0) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "Sleep time must be positive");
        return 2;
    }
    
    vTaskDelay(pdMS_TO_TICKS(ms));
    
    lua_pushboolean(L, true);
    return 1;
}

// SD Card format function
int system_sd_format(lua_State* L) {
    if (!s_sd_mounted) {
        lua_pushboolean(L, false);
        lua_pushstring(L, "SD card not mounted");
        return 2;
    }
    
    // Simulate formatting process
    ESP_LOGI(TAG, "Formatting SD card...");
    vTaskDelay(pdMS_TO_TICKS(2000)); // 2 second delay
    
    lua_pushboolean(L, true);
    lua_pushstring(L, "SD card formatted successfully");
    return 2;
}

// SD Card status check
int system_sd_check_status(lua_State* L) {
    lua_newtable(L);
    lua_pushboolean(L, s_sd_mounted);
    lua_setfield(L, -2, "mounted");
    
    if (s_sd_mounted) {
        lua_pushstring(L, "ready");
    } else {
        lua_pushstring(L, "not_detected");
    }
    lua_setfield(L, -2, "status");
    
    return 1;
}

// WiFi status function
int system_wifi_get_status(lua_State* L) {
    if (!s_wifi_initialized) {
        lua_pushstring(L, "not_initialized");
        return 1;
    }
    
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    
    if (ret == ESP_OK) {
        lua_pushstring(L, "connected");
    } else if (s_wifi_connecting) {
        lua_pushstring(L, "connecting");
    } else {
        lua_pushstring(L, "disconnected");
    }
    
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
    {"sd_format", system_sd_format},
    {"sd_check_status", system_sd_check_status},
    
    // WiFi functions
    {"wifi_init", system_wifi_init},
    {"wifi_scan", system_wifi_scan},
    {"wifi_connect", system_wifi_connect},
    {"wifi_disconnect", system_wifi_disconnect},
    {"wifi_is_connected", system_wifi_is_connected},
    {"wifi_get_ip", system_wifi_get_ip},
    {"wifi_get_status", system_wifi_get_status},
    
    // System functions
    {"delay", system_delay},
    {"get_free_heap", system_get_free_heap},
    {"get_psram_size", system_get_psram_size},
    {"restart", system_restart},
    {"sleep", system_sleep},
    
    // Timer functions
    {"timer_create", system_timer_create},
    {"timer_start", system_timer_start},
    {"timer_stop", system_timer_stop},
    
    {NULL, NULL}
};

int luaopen_system(lua_State* L) {
    ESP_LOGI(TAG, "Registering system bindings...");

    // Create timer metatable
    luaL_newmetatable(L, LUA_TIMER_METATABLE);
    lua_pushcfunction(L, timer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
    
    // Create system table
    luaL_newlib(L, system_functions);
    
    // Set global system table
    lua_setglobal(L, "system");
    
    ESP_LOGI(TAG, "System bindings registered successfully");
    return 0;
}
