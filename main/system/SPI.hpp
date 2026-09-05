#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "memory"

class SDManager;

class SPI final
{
private:
    friend class Display;

public:
    explicit SPI(const spi_host_device_t host, const gpio_num_t mosi, const gpio_num_t miso, const gpio_num_t clk);
    ~SPI();

public:
    static std::unique_ptr<SPI> create_display_spi();
    static std::unique_ptr<SPI> create_sd_spi();

public:
    spi_device_handle_t add_device(const spi_device_interface_config_t& device_conf);
    spi_host_device_t get_host() const;

private:
    const spi_host_device_t _host;
    const gpio_num_t _mosi;
    const gpio_num_t _miso;
    const gpio_num_t _clk;
};
