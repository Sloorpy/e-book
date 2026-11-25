#include "ButtonsArray.hpp"
#include "Config.hpp"
#include "Display.hpp"
#include "Fonts/MyFonts/Serif24.h"

#include <stdexcept>

ButtonsArray::ButtonsArray() :
    _buttons(init_buttons()),
    _current(1),
    _selected_index(std::nullopt)
{
    _buttons[_current].hover();
}

bool ButtonsArray::next()
{
    if (_current + 1 == _buttons.size()) {
        return false;
    }
    _buttons[_current].unhover();
    ++_current;
    _buttons[_current].hover();
    return true;
}

bool ButtonsArray::prev()
{
    if (_current == 0) {
        return false;
    }
    _buttons[_current].unhover();
    --_current;
    _buttons[_current].hover();
    return true;
}

ButtonsArray::Buttons ButtonsArray::init_buttons()
{
    constexpr std::size_t count = std::tuple_size_v<Buttons>;
    constexpr uint16_t top_margin = static_cast<uint16_t>(Config::Home::ROW_HEIGHT * 0.08 + 0.5);
    constexpr uint16_t button_height = Config::Home::ROW_HEIGHT - top_margin;
    constexpr uint16_t step = button_width + button_gap;
    constexpr uint16_t group_width = count * button_width + (count - 1) * button_gap;
    static_assert(group_width <= Config::Display::WIDTH, "Home buttons must fit in the row");

    constexpr uint16_t left = (Config::Display::WIDTH - group_width) / 2;
    constexpr uint16_t middle = left + step;
    constexpr uint16_t right = middle + step;
    return {
        HomeButton("Settings", {{left, top_margin, button_width, button_height}, &Serif24, button_radius}),
        HomeButton("Books", {{middle, top_margin, button_width, button_height}, &Serif24, button_radius}),
        HomeButton("Statistics", {{right, top_margin, button_width, button_height}, &Serif24, button_radius}),
    };
}

void ButtonsArray::draw(CanvasRegion& row)
{
    _buttons[0].draw(row);
    _buttons[1].draw(row);
    _buttons[2].draw(row);
    row.drawFastHLine(0, 0, row.width(), static_cast<uint16_t>(Color::BLACK));
}

bool ButtonsArray::select()
{
    if (_current >= _buttons.size()) {
        throw std::runtime_error("ButtonArray _current is larger then array");
    }

    if (_selected_index.has_value() && _selected_index.value() == _current) {
        return false;
    }

    if (_selected_index.has_value()){
        _buttons[_selected_index.value()].deselect();
    }

    _buttons[_current].select();
    _selected_index = _current;
    return true;
}
