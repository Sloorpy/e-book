#include "Display.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <exception>

static constexpr char TAG[] = "Display";

Display::Display(std::shared_ptr<SPI> spi, const Color screen_mode) :
    GFXcanvas1(_WIDTH, _HEIGHT),
    _spi(spi),
    _handle(initialize_hardware()),
    _mode(screen_mode)
{
    wake_up();        
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

void Display::reset() {
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
}

void Display::waitBusy() {
    constexpr TickType_t POLL_DELAY = pdMS_TO_TICKS(10);
    constexpr uint16_t MAX_POLLS = 1500;
    
    constexpr TickType_t COMMAND_SETTLE_DELAY = pdMS_TO_TICKS(100);
    vTaskDelay(COMMAND_SETTLE_DELAY);

    uint16_t polls = 0;
    while (gpio_get_level(PIN_BUSY) != 0) {
        if (++polls >= MAX_POLLS) {
            ESP_LOGE(TAG, "Display BUSY timeout");
            return;
        }
        vTaskDelay(POLL_DELAY);
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

void Display::sendData16(uint16_t data)
{
    gpio_set_level((gpio_num_t)PIN_DC, 1);
    spi_transaction_t t = {};
    t.length = 16;
    t.tx_buffer = &data;
    ESP_ERROR_CHECK(spi_device_transmit(_handle, &t));
}

void Display::wake_up() {
    reset();
    waitBusy();
    sendCmd(0x12);  // SWRESET
    waitBusy();

    static constexpr uint8_t DARK_MODE = 0x48;
    static constexpr uint8_t LIGHT_MODE = 0x40;
    sendCmd(0x21);
    sendData(_mode == Color::WHITE ? LIGHT_MODE : DARK_MODE);
    sendData(0x00);

    static constexpr uint8_t DARK_BORDER= 0x00;
    static constexpr uint8_t LIGHT_BORDER= 0x05;
    sendCmd(0x3C);
    sendData(_mode == Color::WHITE ? LIGHT_BORDER : DARK_BORDER);

    // Temperatue preparations
    sendCmd(static_cast<uint8_t>(DisplayCommand::TEMPERATURE));
    sendData(0x5A);
    sendCmd(0x22);
    sendData(0x91);
    sendCmd(0x20);
    waitBusy();

    // Ram traveral
    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_TRAVERSAL));
    sendData(0x01);

    
    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_WINDOW_X));
    sendData(0x00); // X start
    sendData(static_cast<uint8_t>(_FRAMEBUFFER_STRIDE - 1)); // X end

    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_WINDOW_Y));
    sendData16(_HEIGHT - 1); // Y start
    sendData16(0); // Y end

    sendCmd(static_cast<uint8_t>(DisplayCommand::X_START_POSITION));
    sendData(0x00);
    sendCmd(static_cast<uint8_t>(DisplayCommand::Y_START_POSITION));
    sendData16(_HEIGHT - 1);
}

void Display::write_ram_region(const DisplayCommand command, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{const uint8_t ram_x_start = static_cast<uint8_t>(x_start / 8);
    const uint8_t ram_x_end = static_cast<uint8_t>(x_end / 8);
    const uint16_t ram_y_start = _HEIGHT - 1 - y_start;
    const uint16_t ram_y_end = _HEIGHT - 1 - y_end;

    // Prepare e-paper to partial update
    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_TRAVERSAL));
    sendData(0x01);

    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_WINDOW_X));
    sendData(ram_x_start);
    sendData(ram_x_end);

    sendCmd(static_cast<uint8_t>(DisplayCommand::RAM_WINDOW_Y));
    sendData16(ram_y_start);
    sendData16(ram_y_end);

    sendCmd(static_cast<uint8_t>(DisplayCommand::X_START_POSITION));
    sendData(ram_x_start);
    
    sendCmd(static_cast<uint8_t>(DisplayCommand::Y_START_POSITION));
    sendData16(ram_y_start);
    

    sendCmd(static_cast<uint8_t>(command));
    gpio_set_level(PIN_DC, 1);

    uint8_t* buffer = getBuffer();
    const uint16_t bytes_per_row = ram_x_end - ram_x_start + 1;
    for (uint16_t row = y_start; row <= y_end; ++row) {
        spi_transaction_t t = {};

        t.length = bytes_per_row * 8;

        t.tx_buffer = buffer + static_cast<size_t>(row) * _FRAMEBUFFER_STRIDE + ram_x_start;

        ESP_ERROR_CHECK(spi_device_transmit(_handle, &t));
    }

    gpio_set_level(PIN_DC, 0);
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

void Display::update()
{
    printf("Display::update()\n");
    wake_up();
    write_ram_region(
        DisplayCommand::RAM_WRITE_PREVIOUS,
        0, 0,
        _WIDTH - 1,
        _HEIGHT - 1
    );

    write_ram_region(
        DisplayCommand::RAM_WRITE,
        0, 0,
        _WIDTH - 1,
        _HEIGHT - 1
    );

    sendCmd(0x22);
    sendData(0xC7);
    sendCmd(0x20);
    waitBusy();
    printf("update complete\n");
}

void Display::full_partial_update()
{
    partial_update(0,0,WIDTH, HEIGHT);
}

void Display::partial_update(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h)
{
    printf("Display::partial_update()\n");

    if (!w || !h ||
        x >= _WIDTH ||
        y >= _HEIGHT ||
        w > _WIDTH - x ||
        h > _HEIGHT - y) {
        throw std::runtime_error("Bad parameters for Display::partial_update");
    }

    const uint16_t x_end = x + w - 1;
    const uint16_t y_end = y + h - 1;    
    write_ram_region(DisplayCommand::RAM_WRITE, x, y, x_end, y_end);

    sendCmd(0x21);
    sendData(0x00);
    sendData(0x00);
    
    sendCmd(0x22);
    sendData(0xFC);
    
    sendCmd(0x20);
    waitBusy();
    write_ram_region(DisplayCommand::RAM_WRITE_PREVIOUS, x, y, x_end, y_end);
    write_ram_region(DisplayCommand::RAM_WRITE, x, y, x_end, y_end);
}

void Display::deep_sleep() {
    waitBusy();
    sendCmd(0x10);
    sendData(0x01);
}

GFXfont* Display::getFont() {
    return gfxFont;
}
