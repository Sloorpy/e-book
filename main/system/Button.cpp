#include "Button.hpp"

#include <esp_timer.h>

Button::Button(gpio_num_t pin,
               uint32_t debounce_ms,
               uint32_t long_press_ms,
               uint32_t double_click_ms)
    : m_pin(pin),
      m_debounce_ms(debounce_ms),
      m_long_press_ms(long_press_ms),
      m_double_click_ms(double_click_ms)
{
}

esp_err_t Button::init()
{
    gpio_config_t io_conf{};
    io_conf.pin_bit_mask = (1ULL << m_pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        return err;
    }

    const bool current_pressed = read_raw_pressed();

    m_last_raw_state = current_pressed;
    m_stable_state = current_pressed;
    m_last_change_time_ms = now_ms();
    m_press_start_time_ms = 0;
    m_long_press_fired = false;
    m_waiting_for_second_click = false;
    m_first_click_time_ms = 0;
    m_pending_event = Event::None;
    m_initialized = true;

    return ESP_OK;
}

void Button::update()
{
    if (!m_initialized) {
        return;
    }

    const uint64_t current_time = now_ms();
    const bool raw_pressed = read_raw_pressed();

    if (raw_pressed != m_last_raw_state) {
        m_last_raw_state = raw_pressed;
        m_last_change_time_ms = current_time;
    }

    const bool debounce_passed =
        (current_time - m_last_change_time_ms) >= m_debounce_ms;

    if (debounce_passed && m_stable_state != m_last_raw_state) {
        m_stable_state = m_last_raw_state;

        if (m_stable_state) {
            // Became pressed
            m_press_start_time_ms = current_time;
            m_long_press_fired = false;
        } else {
            // Became released
            const bool was_short_press = !m_long_press_fired;

            if (was_short_press) {
                if (m_waiting_for_second_click) {
                    const uint64_t delta = current_time - m_first_click_time_ms;

                    if (delta <= m_double_click_ms) {
                        m_waiting_for_second_click = false;
                        m_first_click_time_ms = 0;
                        push_event(Event::DoubleClick);
                    } else {
                        // Previous pending single click expired; emit it and start a new cycle
                        push_event(Event::Click);
                        m_waiting_for_second_click = true;
                        m_first_click_time_ms = current_time;
                    }
                } else {
                    m_waiting_for_second_click = true;
                    m_first_click_time_ms = current_time;
                }
            }

            m_press_start_time_ms = 0;
            m_long_press_fired = false;
        }
    }

    if (m_stable_state && !m_long_press_fired && m_press_start_time_ms != 0) {
        const uint64_t held_time = current_time - m_press_start_time_ms;

        if (held_time >= m_long_press_ms) {
            m_long_press_fired = true;
            m_waiting_for_second_click = false;
            m_first_click_time_ms = 0;
            push_event(Event::LongPress);
        }
    }

    if (m_waiting_for_second_click) {
        const uint64_t wait_time = current_time - m_first_click_time_ms;

        if (wait_time > m_double_click_ms) {
            m_waiting_for_second_click = false;
            m_first_click_time_ms = 0;
            push_event(Event::Click);
        }
    }
}

Button::Event Button::get_state()
{
    Event event = m_pending_event;
    m_pending_event = Event::None;
    return event;
}

bool Button::is_pressed() const
{
    return m_stable_state;
}

uint64_t Button::now_ms() const
{
    return static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);
}

bool Button::read_raw_pressed() const
{
    return gpio_get_level(m_pin) == 0;
}

void Button::push_event(Event event)
{
    m_pending_event = event;
}