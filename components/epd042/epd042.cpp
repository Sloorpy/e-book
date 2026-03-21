#include "epd042.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define EPD_WHITE 0xFFFF

#define PIN_MOSI 23
#define PIN_CLK  18
#define PIN_CS   5
#define PIN_DC   17
#define PIN_RST  16
#define PIN_BUSY 4

Epd042::Epd042() : Adafruit_GFX(EPD042_WIDTH, EPD042_HEIGHT) {
}

void Epd042::init() {
    printf("Epd042 init: SPI pins MOSI=%d CLK=%d CS=%d DC=%d RST=%d BUSY=%d\n",
           PIN_MOSI, PIN_CLK, PIN_CS, PIN_DC, PIN_RST, PIN_BUSY);

    gpio_set_direction((gpio_num_t)PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)PIN_BUSY, GPIO_MODE_INPUT);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = PIN_MOSI;
    buscfg.miso_io_num = -1;
    buscfg.sclk_io_num = PIN_CLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 4000000;
    devcfg.spics_io_num = PIN_CS;
    devcfg.queue_size = 1;
    devcfg.mode = 0;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    fillScreen(EPD_WHITE);
}

void Epd042::reset() {
    gpio_set_level((gpio_num_t)PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Epd042::waitBusy() {
    int count = 0;
    while (gpio_get_level((gpio_num_t)PIN_BUSY) != 0) {
        if (count++ > 2000) {
            printf("waitBusy timeout\n");
            break;
        }
        vTaskDelay(1);
    }
}

void Epd042::sendCmd(uint8_t cmd) {
    gpio_set_level((gpio_num_t)PIN_DC, 0);
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &cmd;
    ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
}

void Epd042::sendData(uint8_t data) {
    gpio_set_level((gpio_num_t)PIN_DC, 1);
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &data;
    ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
}

void Epd042::wakeUp() {
    reset();
    waitBusy();
    sendCmd(0x12);  // SWRESET
    waitBusy();

    sendCmd(0x21);
    sendData(0x40);
    sendData(0x00);

    sendCmd(0x3C);
    sendData(0x05);

    sendCmd(0x1A);
    sendData(0x5A);
    sendCmd(0x22);
    sendData(0x91);
    sendCmd(0x20);
    waitBusy();

    sendCmd(0x11);
    sendData(0x01);

    sendCmd(0x44);
    sendData(0x00);
    sendData(0x31);
    sendCmd(0x45);
    sendData(0x2B);
    sendData(0x01);
    sendData(0x00);
    sendData(0x00);

    sendCmd(0x4E);
    sendData(0x00);
    sendCmd(0x4F);
    sendData(0x2B);
    sendData(0x01);
}

void Epd042::fillScreen(uint16_t color) {
    uint8_t fill = (color == 0) ? 0x00 : 0xFF;
    for (size_t i = 0; i < sizeof(framebuffer); i++) {
        framebuffer[i] = fill;
    }
}

void Epd042::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= EPD042_WIDTH) || (y < 0) || (y >= EPD042_HEIGHT)) return;

    switch (getRotation()) {
        case 1: { int16_t t = x; x = y; y = EPD042_WIDTH - t - 1; break; }
        case 2: x = EPD042_WIDTH - x - 1; y = EPD042_HEIGHT - y - 1; break;
        case 3: { int16_t t = x; x = EPD042_HEIGHT - y - 1; y = t; break; }
    }

    x = EPD042_WIDTH - x - 1;  // mirror X

    uint16_t i = x / 8 + y * (EPD042_WIDTH / 8);
    uint8_t mask = 1 << (7 - (x % 8));

    if (color == 0) {  // BLACK
        framebuffer[i] &= ~mask;
    } else {  // WHITE
        framebuffer[i] |= mask;
    }
}

void Epd042::update() {
    printf("Epd042::update()\n");
    wakeUp();

    sendCmd(0x24);
    gpio_set_level((gpio_num_t)PIN_DC, 1);

    static uint8_t lineBuf[50];
    uint16_t idx = 0;

    for (uint16_t y = 1; y <= EPD042_HEIGHT; y++) {
        for (uint8_t x = 1; x <= 50; x++) {
            lineBuf[x - 1] = (idx < (int)sizeof(framebuffer)) ? framebuffer[idx] : 0xFF;
            idx++;
            if (x == 50) {
                spi_transaction_t t = {};
                t.length = 400;
                t.tx_buffer = lineBuf;
                ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
            }
        }
    }

    sendCmd(0x22);
    sendData(0xC7);
    sendCmd(0x20);
    waitBusy();
    printf("update complete\n");
}

void Epd042::deepsleep() {
    sendCmd(0x10);
    sendData(0x01);
}
