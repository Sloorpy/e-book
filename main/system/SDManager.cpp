#include "SDManager.hpp"
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

SDManager* SDManager::_instance = nullptr;

SDManager& SDManager::instance()
{
    if (!_instance)
    {
        throw std::runtime_error("SDManager not initialized. Call SDManager::init() first.");
    }
    return *_instance;
}

void SDManager::init(std::shared_ptr<SPI> spi, const std::string_view& base_path)
{
    if (_instance)
    {
        throw std::runtime_error("SDManager already initialized");
    }
    _instance = new SDManager(spi, base_path);
}

SDManager::SDManager(std::shared_ptr<SPI> spi, const std::string_view& base_path) :
    _spi(spi),
    _card(mount_sd_card(spi, base_path)),
    _base_path(base_path)
{
    if (!_card)
    {
        throw std::runtime_error("Failed to mount SD card");
    }
}

SDManager::~SDManager()
{
    if (_card)
    {
        esp_vfs_fat_sdcard_unmount(_base_path.data(), _card);
    }
}

std::string_view SDManager::get_base_path() const
{
    return _base_path;
}

sdmmc_card_t *SDManager::mount_sd_card(std::shared_ptr<SPI> spi, const std::string_view& base_path)
{
    static constexpr uint8_t MOUNT_RETRY_COUNT = 3;
    static constexpr uint32_t RETRY_DELAY_MS = 250;
    static constexpr gpio_num_t PIN_SD_MOSI = GPIO_NUM_23;
    static constexpr gpio_num_t PIN_SD_MISO = GPIO_NUM_19;
    static constexpr gpio_num_t PIN_SD_CLK = GPIO_NUM_18;
    static constexpr gpio_num_t PIN_DISPLAY_CS = GPIO_NUM_5;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_PROBING;
    host.command_timeout_ms = 10000;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    host.slot = static_cast<int>(spi->get_host());
    slot_config.gpio_cs = PIN_SD_CS;
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);
    slot_config.wait_for_miso = 127;

    gpio_set_level(PIN_SD_CS, 1);
    gpio_set_direction(PIN_SD_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_DISPLAY_CS, 1);
    gpio_set_direction(PIN_DISPLAY_CS, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(PIN_SD_CS, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_MISO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_CLK, GPIO_PULLUP_ONLY);

    ESP_LOGI(LOG_TAG.data(),
             "SD mount setup host=%d freq=%dKHz sd_cs=%d display_cs=%d",
             host.slot,
             host.max_freq_khz,
             PIN_SD_CS,
             PIN_DISPLAY_CS);

    sdmmc_card_t *card = nullptr;

    for (uint8_t attempt = 1; attempt <= MOUNT_RETRY_COUNT; ++attempt) {
        const esp_err_t ret = esp_vfs_fat_sdspi_mount(base_path.data(), &host, &slot_config, &mount_config, &card);
        if (ret == ESP_OK) {
            ESP_LOGI(LOG_TAG.data(), "SD mount succeeded on attempt %u", attempt);
            return card;
        }

        ESP_LOGW(LOG_TAG.data(),
                 "Mount attempt %u/%u failed: %s",
                 attempt,
                 MOUNT_RETRY_COUNT,
                 esp_err_to_name(ret));

        if (attempt < MOUNT_RETRY_COUNT) {
            vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        }
    }

    ESP_LOGE(LOG_TAG.data(), "Mount failed after %u attempts", MOUNT_RETRY_COUNT);
    return nullptr;
}
