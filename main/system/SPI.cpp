#include "SPI.hpp"
#include <esp_err.h>

SPI::SPI(const spi_host_device_t host) :
    _host(host)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = static_cast<uint8_t>(PIN_MOSI),
        .miso_io_num = static_cast<uint8_t>(PIN_MISO),
        .sclk_io_num = static_cast<uint8_t>(PIN_CLK),
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
