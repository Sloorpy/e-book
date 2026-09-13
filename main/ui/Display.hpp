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

enum class DisplayCommand : uint8_t {
    RAM_TRAVERSAL = 0x11,
    RESET = 0x12,
    TEMPERATURE = 0x1A, 
    BW_UPDATE = 0x21, // Could be Red too (but you don't support it) 
    BW_BOARER_UPDATE = 0x3C,
    RAM_WRITE = 0x24,
    RAM_WRITE_PREVIOUS = 0x26,
    RAM_WINDOW_X = 0x44,
    RAM_WINDOW_Y = 0x45,
    X_START_POSITION = 0x4E,
    Y_START_POSITION = 0x4F,
};


class Display final : public GFXcanvas1
{
public:
    explicit Display(std::shared_ptr<SPI> spi, const Color screen_mode = Color::WHITE);
    ~Display();

public: 
    void update();
    void deep_sleep();
    void fill_screen(const Color color);
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    GFXfont* getFont();

private:
    spi_device_handle_t initialize_hardware();
    void reset();
    void waitBusy();
    void sendCmd(uint8_t cmd);
    void sendData(uint8_t data);
    void sendData16(uint16_t data);
    void wake_up();

private:
    std::shared_ptr<SPI> _spi;
    const spi_device_handle_t _handle;
    Color _mode;

private:
    static constexpr uint16_t _WIDTH  = 400;
    static constexpr uint16_t _HEIGHT = 300;
    static constexpr uint16_t _FRAMEBUFFER_STRIDE = _WIDTH / 8;
    static constexpr gpio_num_t PIN_BUSY =  GPIO_NUM_4;
    static constexpr gpio_num_t PIN_CS   =  GPIO_NUM_5;
    static constexpr gpio_num_t PIN_RST  = GPIO_NUM_16;
    static constexpr gpio_num_t PIN_DC   = GPIO_NUM_17;
};
