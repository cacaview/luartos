#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"        // Add ESP log support
#include "esp_timer.h"      // Add timer support
#include "esp_heap_caps.h"  // Add heap capabilities for PSRAM

#include "lvgl.h"
#include "lvgl_helpers.h"
#include "lua_engine.h"     // Add Lua engine support
#include "main_simple_lua.h" // Add embedded simple Lua script

// Debug tag
static const char *TAG = "MAIN_APP";

#define LV_TICK_PERIOD_MS 1

static void gui_task(void *pvParameter);
void run_lua_demo(void);
void check_gpio_status(void);
void hardware_display_test(void);
void log_memory_usage(const char* stage);

void app_main() {
    ESP_LOGI(TAG, "Application startup");
    ESP_LOGI(TAG, "Available internal memory: %d bytes", esp_get_free_heap_size());
    
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "PSRAM available: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Total PSRAM: %d bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
#else
    ESP_LOGI(TAG, "PSRAM not enabled");
#endif
    
    ESP_LOGI(TAG, "Creating GUI task...");
    
    // Increase stack size for GUI task when using PSRAM
    uint32_t stack_size = 4096 * 3;  // 12KB stack for GUI operations
    BaseType_t result = xTaskCreate(gui_task, "gui", stack_size, NULL, 5, NULL);
    if (result == pdPASS) {
        ESP_LOGI(TAG, "GUI task created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create GUI task: %d", result);
    }
}

#define LV_HOR_RES_MAX 480
#define LV_VER_RES_MAX 320

// Display buffer configuration for PSRAM
// Use larger buffers when PSRAM is available
#ifdef CONFIG_SPIRAM
    #define DISP_BUF_LINES 60  // 60 lines buffer with PSRAM (480 * 60 * 2 = 57.6KB per buffer)
#else
    #define DISP_BUF_LINES 20  // 20 lines buffer for internal RAM fallback
#endif

// Display buffers will be allocated dynamically in PSRAM
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// Global Lua state for memory monitoring
static lua_State* g_lua_state = NULL;

static void gui_task(void *pvParameter) {

    (void) pvParameter;
    
    ESP_LOGI(TAG, "GUI task starting execution");
    ESP_LOGI(TAG, "Task stack remaining: %d bytes", uxTaskGetStackHighWaterMark(NULL));
    
    // Check available memory before initialization
    ESP_LOGI(TAG, "Free heap before LVGL init: %d bytes", esp_get_free_heap_size());
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "Free PSRAM: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Total PSRAM: %d bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
#endif
    
    // Check GPIO status
    check_gpio_status();
    
    // Perform hardware-level display test
    hardware_display_test();
    
    ESP_LOGI(TAG, "Initializing LVGL...");
    lv_init();
    ESP_LOGI(TAG, "LVGL initialization complete");

    /* Initialize SPI or I2C bus used by the drivers */
    ESP_LOGI(TAG, "Initializing LVGL drivers...");
    lvgl_driver_init();
    ESP_LOGI(TAG, "LVGL driver initialization complete");

    // Allocate display buffers in PSRAM for better performance
    uint32_t size_in_px = LV_HOR_RES_MAX * DISP_BUF_LINES;
    uint32_t buf_size_bytes = size_in_px * sizeof(lv_color_t);
    
    ESP_LOGI(TAG, "Allocating display buffers...");
    ESP_LOGI(TAG, "Buffer size: %d pixels (%d bytes per buffer)", size_in_px, buf_size_bytes);

#ifdef CONFIG_SPIRAM
    // Try to allocate in PSRAM first
    buf1 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_SPIRAM);
    if (buf1 == NULL) {
        ESP_LOGW(TAG, "Failed to allocate buf1 in PSRAM, trying internal RAM");
        buf1 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_INTERNAL);
    } else {
        ESP_LOGI(TAG, "buf1 allocated in PSRAM at: %p", buf1);
    }
    
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    buf2 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_SPIRAM);
    if (buf2 == NULL) {
        ESP_LOGW(TAG, "Failed to allocate buf2 in PSRAM, trying internal RAM");
        buf2 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_INTERNAL);
    } else {
        ESP_LOGI(TAG, "buf2 allocated in PSRAM at: %p", buf2);
    }
#endif

#else
    // Fallback to internal RAM
    buf1 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "buf1 allocated in internal RAM at: %p", buf1);
    
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    buf2 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "buf2 allocated in internal RAM at: %p", buf2);
#endif
#endif

    if (buf1 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate display buffer 1!");
        return;
    }

    lv_disp_draw_buf_t disp_buf;
    
    ESP_LOGI(TAG, "Configuring display buffer...");

#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    if (buf2 == NULL) {
        ESP_LOGW(TAG, "Failed to allocate buffer 2, using single buffer mode");
        lv_disp_draw_buf_init(&disp_buf, buf1, NULL, size_in_px);
        ESP_LOGI(TAG, "Single buffer initialization complete");
    } else {
        lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);
        ESP_LOGI(TAG, "Dual buffer initialization complete");
    }
#else
    lv_disp_draw_buf_init(&disp_buf, buf1, NULL, size_in_px);
    ESP_LOGI(TAG, "Single buffer initialization complete");
#endif

    ESP_LOGI(TAG, "Registering display driver...");
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.draw_buf = &disp_buf;
    
    ESP_LOGI(TAG, "Display resolution: %dx%d", disp_drv.hor_res, disp_drv.ver_res);
    ESP_LOGI(TAG, "flush callback address: %p", disp_drv.flush_cb);
    
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    if (disp != NULL) {
        ESP_LOGI(TAG, "Display driver registered successfully");
    } else {
        ESP_LOGE(TAG, "Failed to register display driver");
    }

#if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    ESP_LOGI(TAG, "Initializing touch driver...");
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_t *indev = lv_indev_drv_register(&indev_drv);
    if (indev != NULL) {
        ESP_LOGI(TAG, "Touch driver registered successfully");
    } else {
        ESP_LOGE(TAG, "Failed to register touch driver");
    }
#else
    ESP_LOGW(TAG, "Touch not enabled");
#endif

    ESP_LOGI(TAG, "Starting Lua demo application...");
    
    run_lua_demo();
    ESP_LOGI(TAG, "Lua demo application completed");
    
    // Log memory usage after initialization
    log_memory_usage("After LVGL initialization");
    
    ESP_LOGI(TAG, "Entering main loop");
    uint32_t loop_count = 0;
    uint32_t last_log_time = 0;

    while (1) {
        uint32_t start_time = esp_timer_get_time() / 1000;
        
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
        
        loop_count++;
        
        // Print status information every 5 seconds
        if (start_time - last_log_time > 5000) {
            ESP_LOGI(TAG, "Main loop running - loop count: %d", loop_count);
            log_memory_usage("Main loop status");
            
            // Log Lua memory usage if available
            if (g_lua_state != NULL) {
                size_t total_alloc, psram_alloc, internal_alloc;
                lua_engine_get_memory_stats(g_lua_state, &total_alloc, &psram_alloc, &internal_alloc);
                ESP_LOGI(TAG, "Lua memory: Total=%zu, PSRAM=%zu, Internal=%zu bytes", 
                        total_alloc, psram_alloc, internal_alloc);
            }
            
            ESP_LOGI(TAG, "Task stack remaining: %d bytes", uxTaskGetStackHighWaterMark(NULL));
            last_log_time = start_time;
        }
    }

    vTaskDelete(NULL);
}

void run_lua_demo(void)
{
    ESP_LOGI(TAG, "Initializing Lua engine...");
    
    // Initialize Lua engine with LVGL bindings
    g_lua_state = lua_engine_init();
    if (g_lua_state == NULL) {
        ESP_LOGE(TAG, "Failed to initialize Lua engine");
        return;
    }
    
    ESP_LOGI(TAG, "Lua engine initialized successfully");
    
            // Load and execute embedded simple demo script
    ESP_LOGI(TAG, "Loading embedded simple demo script...");
    int result = lua_engine_exec_string(g_lua_state, main_simple_lua_script);
    if (result != 0) {
        ESP_LOGE(TAG, "Failed to execute embedded Lua script");
        lua_engine_deinit(g_lua_state);
        g_lua_state = NULL;
        return;
    }
    
    ESP_LOGI(TAG, "Lua demo script executed successfully");
    
    // Log memory usage after script execution
    size_t total_alloc, psram_alloc, internal_alloc;
    lua_engine_get_memory_stats(g_lua_state, &total_alloc, &psram_alloc, &internal_alloc);
    
    ESP_LOGI(TAG, "Lua demo initialization completed");
}

void check_gpio_status(void) {
    ESP_LOGI(TAG, "Checking GPIO pin status...");
    
    // Check display-related GPIO pins
    ESP_LOGI(TAG, "Display control pin configuration:");
    ESP_LOGI(TAG, "  MOSI: GPIO %d", 13);  // CONFIG_LV_DISP_SPI_MOSI
    ESP_LOGI(TAG, "  CLK:  GPIO %d", 14);  // CONFIG_LV_DISP_SPI_CLK  
    ESP_LOGI(TAG, "  CS:   GPIO %d", 5);   // CONFIG_LV_DISP_SPI_CS
    ESP_LOGI(TAG, "  DC:   GPIO %d", 16);  // CONFIG_LV_DISP_PIN_DC
    ESP_LOGI(TAG, "  RST:  GPIO %d", 17);  // CONFIG_LV_DISP_PIN_RST
    
    // Read pin status
    ESP_LOGI(TAG, "Pin level status:");
    ESP_LOGI(TAG, "  DC (GPIO16):  %d", gpio_get_level(16));
    ESP_LOGI(TAG, "  RST (GPIO17): %d", gpio_get_level(17));
    ESP_LOGI(TAG, "  CS (GPIO5):   %d", gpio_get_level(5));
    
    ESP_LOGI(TAG, "GPIO status check completed");
}

void hardware_display_test(void) {
    ESP_LOGI(TAG, "Starting hardware-level display test...");
    
    // Direct GPIO pin setup for basic testing
    ESP_LOGI(TAG, "Configuring display control pins...");
    
    // Configure DC pin 
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << 16); // DC pin
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // Configure RST pin
    io_conf.pin_bit_mask = (1ULL << 17); // RST pin
    gpio_config(&io_conf);
    
    // Configure CS pin
    io_conf.pin_bit_mask = (1ULL << 5);  // CS pin
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "Pin configuration completed");
    
    // Test pin switching
    ESP_LOGI(TAG, "Testing pin switching...");
    for (int i = 0; i < 5; i++) {
        gpio_set_level(16, 1); // DC high
        gpio_set_level(17, 1); // RST high
        gpio_set_level(5, 1);  // CS high
        vTaskDelay(pdMS_TO_TICKS(100));
        
        gpio_set_level(16, 0); // DC low
        gpio_set_level(17, 0); // RST low
        gpio_set_level(5, 0);  // CS low
        vTaskDelay(pdMS_TO_TICKS(100));
        
        ESP_LOGI(TAG, "Pin switching iteration %d", i+1);
    }
    
    // Restore normal state
    gpio_set_level(16, 0); // DC low (command mode)
    gpio_set_level(17, 1); // RST high (not in reset)
    gpio_set_level(5, 1);  // CS high (not selected)
    
    ESP_LOGI(TAG, "Hardware test completed");
}

// Memory cleanup function for PSRAM buffers
void cleanup_display_buffers(void) {
    if (buf1 != NULL) {
        free(buf1);
        buf1 = NULL;
        ESP_LOGI(TAG, "Display buffer 1 freed");
    }
    
    if (buf2 != NULL) {
        free(buf2);
        buf2 = NULL;
        ESP_LOGI(TAG, "Display buffer 2 freed");
    }
}

// Memory monitoring function
void log_memory_usage(const char* stage) {
    ESP_LOGI(TAG, "[%s] Free internal heap: %d bytes", stage, esp_get_free_heap_size());
#ifdef CONFIG_SPIRAM
    ESP_LOGI(TAG, "[%s] Free PSRAM: %d bytes", stage, heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
}

