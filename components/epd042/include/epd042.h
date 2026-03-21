#ifndef EPD042_H
#define EPD042_H

#include <stdint.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "Adafruit_GFX.h"

#define EPD042_WIDTH  400
#define EPD042_HEIGHT 300

class Epd042 : public Adafruit_GFX {
public:
    Epd042();
    void init();
    void update();
    void deepsleep();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillScreen(uint16_t color);

private:
    spi_device_handle_t spi;
    uint8_t framebuffer[EPD042_WIDTH * EPD042_HEIGHT / 8];

    void reset();
    void waitBusy();
    void sendCmd(uint8_t cmd);
    void sendData(uint8_t data);
    void wakeUp();
};

#endif
