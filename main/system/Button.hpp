#pragma once

#include <cstdint>
#include <driver/gpio.h>
#include <esp_err.h>
    
class Button
{
public:
    Button(gpio_num_t pin);
    bool is_pressed() const;

private:
    gpio_num_t m_pin;
};