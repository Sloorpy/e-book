#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"

class SDManager;

class SPI final
{
private:
    friend class Display;

public:
    explicit SPI(const spi_host_device_t host=SPI2_HOST);
    ~SPI();

public:
    spi_device_handle_t add_device(const spi_device_interface_config_t& device_conf);
    spi_host_device_t get_host() const;

private:
    const spi_host_device_t _host;

private:
    static constexpr gpio_num_t PIN_MOSI = GPIO_NUM_23;
    static constexpr gpio_num_t PIN_CLK  = GPIO_NUM_18;
    static const gpio_num_t PIN_MISO = GPIO_NUM_19;
};