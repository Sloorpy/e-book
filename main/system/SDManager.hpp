#pragma once
#include "SPI.hpp"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <memory>
#include <dirent.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <string_view>

class SDManager final
{
public:
    static void init(std::shared_ptr<SPI> spi, const std::string_view& base_path = "/sdcard");
    static SDManager& instance();
    SDManager(const SDManager&) = delete;
    SDManager& operator=(const SDManager&) = delete;

public:
    std::string_view get_base_path() const;

private:    
    static sdmmc_card_t* mount_sd_card(std::shared_ptr<SPI> spi, const std::string_view& base_path);

private:
    SDManager(std::shared_ptr<SPI> spi, const std::string_view& base_path);
    ~SDManager();

private:
    std::shared_ptr<SPI> _spi;
    sdmmc_card_t* _card;
    const std::string_view _base_path;
    
private:
    static constexpr std::string_view LOG_TAG = "SDManger";

private:
    static constexpr gpio_num_t PIN_SD_CS = GPIO_NUM_21;
    static SDManager* _instance;
};
