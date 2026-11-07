#include "SPI.hpp"
#include <esp_err.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

SPI::SPI(const spi_host_device_t host,
         const gpio_num_t mosi,
         const gpio_num_t miso,
         const gpio_num_t clk) :
    _host(host),
    _mosi(mosi),
    _miso(miso),
    _clk(clk)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = static_cast<uint8_t>(_mosi),
        .miso_io_num = static_cast<uint8_t>(_miso),
        .sclk_io_num = static_cast<uint8_t>(_clk),
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096 
    };
    
    esp_err_t ret = spi_bus_initialize(_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }
}

SPI::~SPI()
{
    try 
    {
        if (_host)
        {
            spi_bus_free(_host);
        }
    }
    catch(...) {}
}

std::unique_ptr<SPI> SPI::create_display_spi()
{
    return std::make_unique<SPI>(SPI2_HOST, GPIO_NUM_23, GPIO_NUM_19, GPIO_NUM_18);
}

std::unique_ptr<SPI> SPI::create_sd_spi()
{
    static constexpr gpio_num_t PIN_SD_MOSI = GPIO_NUM_13;
    static constexpr gpio_num_t PIN_SD_MISO = GPIO_NUM_33;
    static constexpr gpio_num_t PIN_SD_CLK = GPIO_NUM_14;
    static constexpr TickType_t SIGNAL_SETTLE_DELAY = pdMS_TO_TICKS(10);

    gpio_set_pull_mode(PIN_SD_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_MISO, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_SD_CLK, GPIO_PULLUP_ONLY);
    vTaskDelay(SIGNAL_SETTLE_DELAY);

    return std::make_unique<SPI>(SPI3_HOST, PIN_SD_MOSI, PIN_SD_MISO, PIN_SD_CLK);
}

spi_device_handle_t SPI::add_device(const spi_device_interface_config_t& device_conf)
{
    spi_device_handle_t handle = nullptr;
    ESP_ERROR_CHECK(spi_bus_add_device(_host, &device_conf, &handle));
    return handle;
}

spi_host_device_t SPI::get_host() const
{
    return _host;
}
