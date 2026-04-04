#pragma once

#include <cstdint>
#include <driver/gpio.h>
#include <esp_err.h>

class Button
{
public:
    enum class Event
    {
        None,
        Click,
        DoubleClick,
        LongPress
    };

    Button(gpio_num_t pin,
           uint32_t debounce_ms = 30,
           uint32_t long_press_ms = 450,
           uint32_t double_click_ms = 600);

    esp_err_t init();
    void update();

    Event get_state();
    bool is_pressed() const;

private:
    uint64_t now_ms() const;
    bool read_raw_pressed() const;
    void push_event(Event event);

private:
    gpio_num_t m_pin;
    uint32_t m_debounce_ms;
    uint32_t m_long_press_ms;
    uint32_t m_double_click_ms;

    bool m_initialized = false;

    bool m_last_raw_state = false;
    bool m_stable_state = false;

    uint64_t m_last_change_time_ms = 0;
    uint64_t m_press_start_time_ms = 0;

    bool m_long_press_fired = false;

    bool m_waiting_for_second_click = false;
    uint64_t m_first_click_time_ms = 0;

    Event m_pending_event = Event::None;
};