#pragma once
#include "SPI.hpp"
#include "Adafruit_GFX.h"

#include <stdint.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <memory>

struct Vector2 {
    int16_t x;
    int16_t y;
};

enum class Color : uint8_t 
{
    BLACK = 0,
    WHITE = 1
};

class Display final : public GFXcanvas1
{
public:
    explicit Display(std::shared_ptr<SPI> spi);
    ~Display();

public: 
    void update();
    void deep_sleep();
    void fill_screen(const Color color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    GFXfont* getFont();

private:
    spi_device_handle_t initialize_hardware();
    void reset();
    void waitBusy();
    void sendCmd(uint8_t cmd);
    void sendData(uint8_t data);
    void wakeUp();

private:
    std::shared_ptr<SPI> _spi;
    const spi_device_handle_t _handle;

private:
    static constexpr uint16_t _WIDTH  = 400;
    static constexpr uint16_t _HEIGHT = 300;
    static constexpr uint16_t _FRAMEBUFFER_STRIDE = _WIDTH / 8;
    static constexpr gpio_num_t PIN_BUSY =  GPIO_NUM_4;
    static constexpr gpio_num_t PIN_CS   =  GPIO_NUM_5;
    static constexpr gpio_num_t PIN_RST  = GPIO_NUM_16;
    static constexpr gpio_num_t PIN_DC   = GPIO_NUM_17;
};