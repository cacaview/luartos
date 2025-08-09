#ifndef SDCARD_DRIVER_H
#define SDCARD_DRIVER_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

// SD card configuration
#define SDCARD_MOUNT_POINT "/sdcard"
#define SDCARD_SPI_HOST SPI3_HOST
#define SDCARD_MAX_TRANSFER_SIZE 4096   // 4KB, a more common and compatible size
#define SDCARD_CS_GPIO GPIO_NUM_9
#define SDCARD_SCK_GPIO GPIO_NUM_11
#define SDCARD_MOSI_GPIO GPIO_NUM_42
#define SDCARD_MISO_GPIO GPIO_NUM_41

// SD card driver structure
typedef struct {
    sdmmc_card_t* card;
    bool mounted;
    size_t max_files;
    char mount_point[64];  // Fixed size buffer instead of pointer
} sdcard_config_t;

/**
 * @brief Initialize SD card driver
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_init(void);

/**
 * @brief Mount SD card with FAT filesystem
 * @param mount_point Mount point (default: "/sdcard")
 * @param max_files Maximum number of files (default: 5)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_mount(const char* mount_point, size_t max_files);

/**
 * @brief Unmount SD card
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_unmount(void);

/**
 * @brief Get SD card information
 * @param info Pointer to store card info
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_get_info(sdmmc_card_t** info);

/**
 * @brief Format SD card with FAT32 filesystem
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_format(void);

/**
 * @brief Check if SD card is mounted
 * @return bool true if mounted
 */
bool sdcard_is_mounted(void);

/**
 * @brief Read file from SD card
 * @param filename File path
 * @param buffer Buffer to store data
 * @param size Buffer size
 * @return int Number of bytes read, -1 on error
 */
int sdcard_read_file(const char* filename, char* buffer, size_t size);

/**
 * @brief Write file to SD card
 * @param filename File path
 * @param data Data to write
 * @param size Data size
 * @return int Number of bytes written, -1 on error
 */
int sdcard_write_file(const char* filename, const char* data, size_t size);

/**
 * @brief Delete file from SD card
 * @param filename File path
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_delete_file(const char* filename);

/**
 * @brief List files in directory
 * @param path Directory path
 * @param callback Callback function for each entry (file or directory)
 * @param user_data User data passed to callback
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_list_files(const char* path, void (*callback)(const char* filename, uint8_t type, size_t size, void* user_data), void* user_data);

/**
 * @brief Get disk usage information
 * @param total_bytes Total bytes on disk
 * @param used_bytes Used bytes on disk
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sdcard_get_usage(uint64_t* total_bytes, uint64_t* used_bytes);

#ifdef __cplusplus
}
#endif

#endif // SDCARD_DRIVER_H
