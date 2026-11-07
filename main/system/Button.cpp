#include "Button.hpp"

#include <esp_timer.h>

Button::Button(gpio_num_t pin)
    : m_pin(pin)
{
}

bool Button::is_pressed() const
{
    return gpio_get_level(m_pin) == 0;;
}