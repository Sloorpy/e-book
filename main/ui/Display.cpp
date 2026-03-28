#include "Display.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Fonts/hebEng5x7avia.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include "HebrewHelper.hpp"

Display::Display(std::shared_ptr<SPI> spi) :
    GFXcanvas1(_WIDTH, _HEIGHT),
    _spi(spi),
    _handle(initialize_hardware())
{
    initialize_font();
    wakeUp();        
    fill_screen(Color::WHITE);
    setRotation(1);
}

Display::~Display()
{
    try 
    {
        if (_handle) {
            const esp_err_t ret = spi_bus_remove_device(_handle);
            if (ret != ESP_OK) {
                ESP_LOGW("Display", "spi_bus_remove_device failed: %s", esp_err_to_name(ret));
            }
        }
    }
    catch (...) {}
}

spi_device_handle_t Display::initialize_hardware()
{
    gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_BUSY, GPIO_MODE_INPUT);

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 4000000;
    devcfg.spics_io_num = PIN_CS;
    devcfg.queue_size = 1;
    devcfg.mode = 0;

    return _spi->add_device(devcfg);
}

void Display::initialize_font()
{
    setTextColor(static_cast<uint8_t>(Color::BLACK));
    setFont(&hebEng5x7avia);

    static constexpr uint16_t HEBREW_START_X = 300;
    static constexpr uint16_t HEBREW_START_Y = 0;
    setCursor(HEBREW_START_X, HEBREW_START_Y);
    setTextSize(2);
}

void Display::reset() {
    gpio_set_level((gpio_num_t)PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level((gpio_num_t)PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Display::waitBusy() {
    int count = 0;
    while (gpio_get_level((gpio_num_t)PIN_BUSY) != 0) {
        if (count++ > 2000) {
            printf("waitBusy timeout\n");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void Display::sendCmd(uint8_t cmd) {
    gpio_set_level((gpio_num_t)PIN_DC, 0);
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &cmd;
    ESP_ERROR_CHECK(spi_device_transmit(_handle, &t));
}

void Display::sendData(uint8_t data) {
    gpio_set_level((gpio_num_t)PIN_DC, 1);
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &data;
    ESP_ERROR_CHECK(spi_device_transmit(_handle, &t));
}

void Display::wakeUp() {
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

void Display::fill_screen(const Color color)
{
    GFXcanvas1::fillScreen(static_cast<uint8_t>(color));
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // logical rotated bounds
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) {
        return;
    }

    // map logical rotated coordinates -> raw framebuffer coordinates
    int16_t t;
    switch (rotation) {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;

        case 2:
            x = WIDTH  - 1 - x;
            y = HEIGHT - 1 - y;
            break;

        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;

        case 0:
        default:
            break;
    }

    // panel/framebuffer X axis is mirrored relative to logical canvas
    x = WIDTH - 1 - x;

    // write to raw 1-bit framebuffer
    uint8_t* buffer = getBuffer();
    const size_t index = static_cast<size_t>(x / 8) +
                         static_cast<size_t>(y) * _FRAMEBUFFER_STRIDE;
    const uint8_t mask = static_cast<uint8_t>(0x80 >> (x & 7));

    if (color == static_cast<uint16_t>(Color::BLACK)) {
        buffer[index] &= static_cast<uint8_t>(~mask);
    } else {
        buffer[index] |= mask;
    }
    
}
GFXfont *Display::get_font() const
{
    return gfxFont;
}

void Display::update()
{
    printf("Display::update()\n");
    wakeUp();

    sendCmd(0x24);
    gpio_set_level((gpio_num_t)PIN_DC, 1);

        uint8_t* buffer = getBuffer();

    for (uint16_t y = 0; y < _HEIGHT; y++) {
        spi_transaction_t t = {};
        t.length = _FRAMEBUFFER_STRIDE * 8;
        t.tx_buffer = buffer + static_cast<size_t>(y) * _FRAMEBUFFER_STRIDE;
        ESP_ERROR_CHECK(spi_device_transmit(_handle, &t));
    }

    sendCmd(0x22);
    sendData(0xC7);
    sendCmd(0x20);
    waitBusy();
    printf("update complete\n");
}

void Display::deep_sleep() {
    sendCmd(0x10);
    sendData(0x01);
}

void Display::writeHebrew(uint8_t letter, bool is_rtl) {
    if (letter == '\r') {
        return;
    }

    const uint8_t first = gfxFont->first;
    const uint8_t last = gfxFont->last;
    if (letter != '\n' && (letter < first || letter > last)) {
        return;
    }

    if (letter != '\n') {
        const GFXglyph* const glyph = gfxFont->glyph + letter - first;
        if (glyph->width == 0 || glyph->height == 0) {
            return;
        }
    }

    const CursorCalculation calc = HebrewHelper::calculateCursor(*this, letter, is_rtl);

    if (letter != '\n') {
        drawChar(calc.draw_x, calc.draw_y, letter, textcolor, textbgcolor, textsize_x, textsize_y);
    }

    cursor_x = calc.next_x;
    cursor_y = calc.next_y;
}

void Display::print_hebrew(const char* str) {
    const std::vector<FontIndex> chars = HebrewHelper::process(str);
    for (const FontIndex& ch : chars) {
        writeHebrew(ch.index, ch.is_rtl);
    }
}

void Display::hebrew_screen()
{
    setFont(&hebEng5x7avia);
    setTextSize(2);
    setTextColor(static_cast<uint8_t>(Color::BLACK));
    static constexpr uint16_t Y_AXIS = 2;
    setCursor(width(), Y_AXIS);
}
