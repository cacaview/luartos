#include "sdcard_driver.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "ff.h"  // FatFs library
#include "hal/spi_types.h"
#include <dirent.h>

static const char *TAG = "SDCARD";
static sdcard_config_t g_sdcard_config = {0};
static bool spi_bus_initialized = false;

esp_err_t sdcard_init(void)
{
    ESP_LOGI(TAG, "Initializing SD card");
    
    // Check if SPI bus is already initialized
    if (!spi_bus_initialized) {
        // Initialize SPI bus
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SDCARD_MOSI_GPIO,
            .miso_io_num = SDCARD_MISO_GPIO,
            .sclk_io_num = SDCARD_SCK_GPIO,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = SDCARD_MAX_TRANSFER_SIZE,  // Use defined constant for better maintenance
        };
        
        esp_err_t ret = spi_bus_initialize(SDCARD_SPI_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
            return ret;
        }
        
        ESP_LOGI(TAG, "SD card SPI bus initialized with max_transfer_sz: %d bytes", bus_cfg.max_transfer_sz);
        
        spi_bus_initialized = true;
        ESP_LOGI(TAG, "SD card SPI bus initialized");
    } else {
        ESP_LOGI(TAG, "SD card SPI bus already initialized");
    }
    
    return ESP_OK;
}

esp_err_t sdcard_mount(const char* mount_point, size_t max_files)
{
    if (g_sdcard_config.mounted) {
        ESP_LOGW(TAG, "SD card already mounted");
        return ESP_OK;
    }
    
    if (mount_point == NULL) {
        mount_point = SDCARD_MOUNT_POINT;
    }
    if (max_files == 0) {
        max_files = 5;
    }
    
    ESP_LOGI(TAG, "Mounting SD card at %s", mount_point);
    
    // Configure SD/MMC host for SPI protocol with conservative settings
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SDCARD_SPI_HOST;
    host.max_freq_khz = 1000;       // Reduced from 400 to 1MHz for stability
    host.io_voltage = 3.3f;         // Explicitly set voltage
    host.init = &sdspi_host_init;   // Use SPI-specific init
    host.set_bus_width = NULL;      // SPI doesn't support bus width changes
    host.get_bus_width = NULL;      // SPI doesn't support bus width changes
    host.set_bus_ddr_mode = NULL;   // SPI doesn't support DDR mode
    host.set_card_clk = &sdspi_host_set_card_clk;
    host.do_transaction = &sdspi_host_do_transaction;
    host.deinit = &sdspi_host_deinit;
    
    // Configure SPI device with explicit parameters  
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SDCARD_CS_GPIO;
    slot_config.host_id = SDCARD_SPI_HOST;
    slot_config.gpio_cd = GPIO_NUM_NC;    // No card detect
    slot_config.gpio_wp = GPIO_NUM_NC;    // No write protect
    slot_config.gpio_int = GPIO_NUM_NC;   // No interrupt
    
    // Mount filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = max_files,
        .allocation_unit_size = 0       // Use default allocation unit size for better compatibility
    };
    
    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }
    
    // Save configuration
    g_sdcard_config.card = card;
    g_sdcard_config.mounted = true;
    g_sdcard_config.max_files = max_files;
    strncpy(g_sdcard_config.mount_point, mount_point, sizeof(g_sdcard_config.mount_point) - 1);
    g_sdcard_config.mount_point[sizeof(g_sdcard_config.mount_point) - 1] = '\0';
    
    // Print card info
    ESP_LOGI(TAG, "SD card mounted successfully");
    ESP_LOGI(TAG, "Name: %s", card->cid.name);
    ESP_LOGI(TAG, "Type: %s", (card->ocr & (1 << 30)) ? "SDHC/SDXC" : "SDSC");  // Bit 30 indicates SDHC
    ESP_LOGI(TAG, "Speed: %s", (card->csd.tr_speed > 25000000) ? "high speed" : "default speed");
    ESP_LOGI(TAG, "Size: %lluMB", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    
    return ESP_OK;
}

esp_err_t sdcard_unmount(void)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGW(TAG, "SD card not mounted");
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Unmounting SD card");
    
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(g_sdcard_config.mount_point, g_sdcard_config.card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unmount SD card: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Free SPI bus
    spi_bus_free(SDCARD_SPI_HOST);
    spi_bus_initialized = false;
    
    // Clear configuration
    memset(&g_sdcard_config, 0, sizeof(g_sdcard_config));
    
    ESP_LOGI(TAG, "SD card unmounted successfully");
    return ESP_OK;
}

esp_err_t sdcard_get_info(sdmmc_card_t** info)
{
    if (!g_sdcard_config.mounted || !g_sdcard_config.card) {
        return ESP_ERR_INVALID_STATE;
    }
    
    *info = g_sdcard_config.card;
    return ESP_OK;
}

esp_err_t sdcard_format(void)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Formatting SD card...");
    
    // Unmount first
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(g_sdcard_config.mount_point, g_sdcard_config.card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unmount before format: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Remount with format option
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SDCARD_SPI_HOST;
    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SDCARD_CS_GPIO;
    slot_config.host_id = SDCARD_SPI_HOST;
    
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,  // Enable formatting
        .max_files = g_sdcard_config.max_files,
        .allocation_unit_size = 0        // Use default allocation unit size
    };
    
    sdmmc_card_t *card;
    ret = esp_vfs_fat_sdspi_mount(g_sdcard_config.mount_point, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to format SD card: %s", esp_err_to_name(ret));
        g_sdcard_config.mounted = false;
        return ret;
    }
    
    g_sdcard_config.card = card;
    ESP_LOGI(TAG, "SD card formatted successfully");
    
    return ESP_OK;
}

bool sdcard_is_mounted(void)
{
    return g_sdcard_config.mounted;
}

int sdcard_read_file(const char* filename, char* buffer, size_t size)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return -1;
    }
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", g_sdcard_config.mount_point, filename);
    
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file %s", filepath);
        return -1;
    }
    
    size_t bytes_read = fread(buffer, 1, size - 1, f);
    buffer[bytes_read] = '\0';  // Null terminate
    
    fclose(f);
    ESP_LOGI(TAG, "Read %d bytes from %s", bytes_read, filename);
    
    return bytes_read;
}

int sdcard_write_file(const char* filename, const char* data, size_t size)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return -1;
    }
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", g_sdcard_config.mount_point, filename);
    
    FILE *f = fopen(filepath, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to create file %s", filepath);
        return -1;
    }
    
    size_t bytes_written = fwrite(data, 1, size, f);
    fclose(f);
    
    ESP_LOGI(TAG, "Wrote %d bytes to %s", bytes_written, filename);
    return bytes_written;
}

esp_err_t sdcard_delete_file(const char* filename)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", g_sdcard_config.mount_point, filename);
    
    if (unlink(filepath) != 0) {
        ESP_LOGE(TAG, "Failed to delete file %s", filename);
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Deleted file %s", filename);
    return ESP_OK;
}

esp_err_t sdcard_list_files(const char* path, void (*callback)(const char* filename, uint8_t type, size_t size, void* user_data), void* user_data)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    char dirpath[256];
    if (path[0] == '/') {
        snprintf(dirpath, sizeof(dirpath), "%s%s", g_sdcard_config.mount_point, path);
    } else {
        snprintf(dirpath, sizeof(dirpath), "%s/%s", g_sdcard_config.mount_point, path);
    }
    
    DIR *dir = opendir(dirpath);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory %s", dirpath);
        return ESP_FAIL;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignore "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t size = 0;
        // Get file size only for regular files
        if (entry->d_type == DT_REG) {
            struct stat st;
            char fullpath[512];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
            if (stat(fullpath, &st) == 0) {
                size = st.st_size;
            }
        }

        if (callback) {
            callback(entry->d_name, entry->d_type, size, user_data);
        }
    }
    
    closedir(dir);
    return ESP_OK;
}

esp_err_t sdcard_get_usage(uint64_t* total_bytes, uint64_t* used_bytes)
{
    if (!g_sdcard_config.mounted) {
        ESP_LOGE(TAG, "SD card not mounted");
        return ESP_ERR_INVALID_STATE;
    }
    
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    
    // Get volume information and free cluster count
    esp_err_t ret = f_getfree("0:", &fre_clust, &fs);
    if (ret != FR_OK) {
        ESP_LOGE(TAG, "Failed to get filesystem info");
        return ESP_FAIL;
    }
    
    // Get total sectors and free sectors
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    
    // Convert to bytes
    *total_bytes = tot_sect * fs->ssize;
    *used_bytes = *total_bytes - (fre_sect * fs->ssize);
    
    return ESP_OK;
}
